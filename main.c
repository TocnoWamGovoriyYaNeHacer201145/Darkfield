#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

// Types

typedef enum {
    TOK_EOF, TOK_ID, TOK_NUM, TOK_STR,
    TOK_PLUS, TOK_MINUS, TOK_STAR, TOK_SLASH,
    TOK_LT, TOK_GT, TOK_EQEQ, TOK_NEQ,
    TOK_EQ, 
    TOK_LPAREN, TOK_RPAREN, TOK_LBRACE, TOK_RBRACE, 
    TOK_COMMA, TOK_SEMICOLON,
    TOK_KW_FUNC, TOK_KW_SHOW, TOK_KW_IF, TOK_KW_ELSE, 
    TOK_KW_RETURN, TOK_KW_WHILE, TOK_KW_FOR, TOK_KW_IMPORT
} TokenType;

typedef struct { TokenType type; char* value; int line; } Token;
typedef struct { char* src; int pos; int line; } Lexer;

typedef enum {
    NODE_PROG, NODE_BLOCK,
    NODE_FUNC_DEF, NODE_FUNC_CALL, NODE_IMPORT,
    NODE_IF, NODE_WHILE, NODE_FOR, NODE_RETURN,
    NODE_ASSIGN, NODE_BIN_OP,
    NODE_NUM, NODE_STR, NODE_ID, NODE_SHOW
} NodeType;

typedef struct ASTNode {
    NodeType type;
    struct ASTNode *left, *right, *extra;
    char* value;
} ASTNode;

typedef struct { Lexer* l; Token* curr; } Parser;

// Lexer

Lexer* create_lexer(char* src) {
    Lexer* l = malloc(sizeof(Lexer)); l->src = src; l->pos = 0; l->line = 1; return l;
}

Token* mk_tok(TokenType t, char* v, int l) {
    Token* tok = malloc(sizeof(Token)); tok->type=t; tok->value=v?strdup(v):NULL; tok->line=l; return tok;
}

TokenType get_kw(char* s) {
    if(!strcmp(s,"func")) return TOK_KW_FUNC;
    if(!strcmp(s,"show")) return TOK_KW_SHOW;
    if(!strcmp(s,"return")) return TOK_KW_RETURN;
    if(!strcmp(s,"if")) return TOK_KW_IF;
    if(!strcmp(s,"else")) return TOK_KW_ELSE;
    if(!strcmp(s,"while")) return TOK_KW_WHILE;
    if(!strcmp(s,"for")) return TOK_KW_FOR;
    if(!strcmp(s,"import")) return TOK_KW_IMPORT;
    return TOK_ID;
}

Token* next_tok(Lexer* l) {
    while(l->src[l->pos]) {
        char c = l->src[l->pos];
        if(isspace(c)) { if(c=='\n') l->line++; l->pos++; continue; }
        // Комментарии //
        if(c=='/' && l->src[l->pos+1]=='/') { 
            while(l->src[l->pos] && l->src[l->pos]!='\n') l->pos++; 
            continue; 
        }
        break;
    }
    if(!l->src[l->pos]) return mk_tok(TOK_EOF, 0, l->line);

    char c = l->src[l->pos];
    if(isalpha(c) || c=='_') {
        int start = l->pos;
        while(isalnum(l->src[l->pos]) || l->src[l->pos]=='_') l->pos++;
        int len = l->pos - start;
        char* v = malloc(len+1); strncpy(v, l->src+start, len); v[len]=0;
        return mk_tok(get_kw(v), v, l->line);
    }
    if(isdigit(c)) {
        int start = l->pos;
        while(isdigit(l->src[l->pos])) l->pos++;
        char* v = malloc(l->pos - start + 1); strncpy(v, l->src+start, l->pos-start); v[l->pos-start]=0;
        return mk_tok(TOK_NUM, v, l->line);
    }
    if(c=='"' || c=='\'') {
        char q=c; l->pos++; int start=l->pos;
        while(l->src[l->pos] && l->src[l->pos]!=q) { if(l->src[l->pos]=='\n') l->line++; l->pos++; }
        char* v = malloc(l->pos - start + 1); strncpy(v, l->src+start, l->pos-start); v[l->pos-start]=0;
        if(l->src[l->pos]==q) l->pos++; 
        return mk_tok(TOK_STR, v, l->line);
    }

    l->pos++;
    switch(c) {
        case '+': return mk_tok(TOK_PLUS, "+", l->line);
        case '-': return mk_tok(TOK_MINUS, "-", l->line);
        case '*': return mk_tok(TOK_STAR, "*", l->line);
        case '/': return mk_tok(TOK_SLASH, "/", l->line);
        case '(': return mk_tok(TOK_LPAREN, "(", l->line);
        case ')': return mk_tok(TOK_RPAREN, ")", l->line);
        case '{': return mk_tok(TOK_LBRACE, "{", l->line);
        case '}': return mk_tok(TOK_RBRACE, "}", l->line);
        case ';': return mk_tok(TOK_SEMICOLON, ";", l->line);
        case ',': return mk_tok(TOK_COMMA, ",", l->line);
        case '<': return mk_tok(TOK_LT, "<", l->line);
        case '>': return mk_tok(TOK_GT, ">", l->line);
        case '!': if(l->src[l->pos]=='=') { l->pos++; return mk_tok(TOK_NEQ, "!=", l->line); } break;
        case '=': if(l->src[l->pos]=='=') { l->pos++; return mk_tok(TOK_EQEQ, "==", l->line); }
                  return mk_tok(TOK_EQ, "=", l->line);
    }
    printf("Lexer Error: Unknown char '%c' at line %d\n", c, l->line); exit(1);
}

// Parser

ASTNode* mk_node(NodeType t, ASTNode* l, ASTNode* r, ASTNode* x, char* v) {
    ASTNode* n = malloc(sizeof(ASTNode)); n->type=t; n->left=l; n->right=r; n->extra=x; n->value=v?strdup(v):NULL; return n;
}

Parser* create_parser(Lexer* l) { Parser* p=malloc(sizeof(Parser)); p->l=l; p->curr=next_tok(l); return p; }

void eat(Parser* p, TokenType t) {
    if(p->curr->type == t) { if(p->curr->value) free(p->curr->value); free(p->curr); p->curr=next_tok(p->l); }
    else { printf("Syntax Error line %d: expected %d got %d val='%s'\n", p->curr->line, t, p->curr->type, p->curr->value); exit(1); }
}

ASTNode* parse_expr(Parser* p);
ASTNode* parse_block(Parser* p);

ASTNode* parse_primary(Parser* p) {
    if(p->curr->type==TOK_NUM) { ASTNode* n=mk_node(NODE_NUM,0,0,0,p->curr->value); eat(p,TOK_NUM); return n; }
    if(p->curr->type==TOK_STR) { ASTNode* n=mk_node(NODE_STR,0,0,0,p->curr->value); eat(p,TOK_STR); return n; }
    if(p->curr->type==TOK_ID) {
        char* name = strdup(p->curr->value); eat(p,TOK_ID);
        if(p->curr->type==TOK_LPAREN) { // Function call
            eat(p,TOK_LPAREN); ASTNode* args=NULL, *curr=NULL;
            while(p->curr->type!=TOK_RPAREN) {
                ASTNode* expr = parse_expr(p);
                ASTNode* arg = mk_node(NODE_BLOCK, expr, NULL, NULL, NULL); 
                if(!args) args=arg; else curr->right=arg; curr=arg;
                if(p->curr->type==TOK_COMMA) eat(p,TOK_COMMA);
            }
            eat(p,TOK_RPAREN); return mk_node(NODE_FUNC_CALL, args, NULL, NULL, name);
        }
        return mk_node(NODE_ID,0,0,0,name);
    }
    if(p->curr->type==TOK_LPAREN) { eat(p,TOK_LPAREN); ASTNode* e=parse_expr(p); eat(p,TOK_RPAREN); return e; }
    return NULL;
}

ASTNode* parse_term(Parser* p) {
    ASTNode* left = parse_primary(p);
    while(p->curr->type == TOK_STAR || p->curr->type == TOK_SLASH) {
        char* op = strdup(p->curr->value); eat(p, p->curr->type);
        left = mk_node(NODE_BIN_OP, left, parse_primary(p), NULL, op);
        free(op);
    }
    return left;
}

ASTNode* parse_additive(Parser* p) {
    ASTNode* left = parse_term(p);
    while(p->curr->type == TOK_PLUS || p->curr->type == TOK_MINUS) {
        char* op = strdup(p->curr->value); eat(p, p->curr->type);
        left = mk_node(NODE_BIN_OP, left, parse_term(p), NULL, op);
        free(op);
    }
    return left;
}

ASTNode* parse_relational(Parser* p) {
    ASTNode* left = parse_additive(p);
    while(p->curr->type == TOK_LT || p->curr->type == TOK_GT) {
        char* op = strdup(p->curr->value); eat(p, p->curr->type);
        left = mk_node(NODE_BIN_OP, left, parse_additive(p), NULL, op);
        free(op);
    }
    return left;
}

ASTNode* parse_expr(Parser* p) {
    ASTNode* left = parse_relational(p);
    while(p->curr->type == TOK_EQEQ || p->curr->type == TOK_NEQ) {
        char* op = strdup(p->curr->value); eat(p, p->curr->type);
        left = mk_node(NODE_BIN_OP, left, parse_relational(p), NULL, op);
        free(op);
    }
    if(left && left->type==NODE_ID && p->curr->type==TOK_EQ) {
        char* name = strdup(left->value); eat(p,TOK_EQ);
        return mk_node(NODE_ASSIGN, NULL, parse_expr(p), NULL, name);
    }
    return left;
}

ASTNode* parse_import(Parser* p) {
    eat(p, TOK_KW_IMPORT);
    char* path = strdup(p->curr->value); eat(p, TOK_STR); eat(p, TOK_SEMICOLON);
    return mk_node(NODE_IMPORT, NULL, NULL, NULL, path);
}

ASTNode* parse_block(Parser* p) {
    eat(p, TOK_LBRACE);
    ASTNode *head=NULL, *curr=NULL;
    while(p->curr->type!=TOK_RBRACE && p->curr->type!=TOK_EOF) {
        ASTNode* stmt = NULL;
        if(p->curr->type==TOK_KW_SHOW) { eat(p,TOK_KW_SHOW); eat(p,TOK_LPAREN); stmt=mk_node(NODE_SHOW, parse_expr(p),0,0,0); eat(p,TOK_RPAREN); }
        else if(p->curr->type==TOK_KW_IF) {
            eat(p,TOK_KW_IF); eat(p,TOK_LPAREN); ASTNode* c=parse_expr(p); eat(p,TOK_RPAREN);
            ASTNode* th=parse_block(p);
            ASTNode* el=(p->curr->type==TOK_KW_ELSE)?(eat(p,TOK_KW_ELSE),parse_block(p)):NULL;
            stmt=mk_node(NODE_IF,c,th,el,NULL);
        }
        else if(p->curr->type==TOK_KW_WHILE) {
            eat(p,TOK_KW_WHILE); eat(p,TOK_LPAREN); ASTNode* c=parse_expr(p); eat(p,TOK_RPAREN);
            stmt=mk_node(NODE_WHILE,c,parse_block(p),0,0);
        }
        else if(p->curr->type==TOK_KW_FOR) {
            eat(p,TOK_KW_FOR); eat(p,TOK_LPAREN);
            ASTNode* init=parse_expr(p); eat(p,TOK_SEMICOLON);
            ASTNode* cond=parse_expr(p); eat(p,TOK_SEMICOLON);
            ASTNode* step=parse_expr(p); eat(p,TOK_RPAREN);
            stmt=mk_node(NODE_FOR, init, cond, mk_node(NODE_BLOCK, step, parse_block(p),0,0), NULL);
        }
        else if(p->curr->type==TOK_KW_RETURN) {
            eat(p,TOK_KW_RETURN); stmt=mk_node(NODE_RETURN, (p->curr->type!=TOK_SEMICOLON)?parse_expr(p):0,0,0,0);
        }
        else if(p->curr->type==TOK_KW_IMPORT) stmt = parse_import(p);
        else stmt = parse_expr(p);
        ASTNode* n = mk_node(NODE_BLOCK, stmt, NULL, NULL, NULL);
        if(!head) head=n; else curr->right=n; curr=n;
        if(p->curr->type==TOK_SEMICOLON) eat(p,TOK_SEMICOLON);
    }
    eat(p, TOK_RBRACE);
    return head;
}

ASTNode* parse_prog(Parser* p) {
    ASTNode *head=NULL, *curr=NULL;
    while(p->curr->type!=TOK_EOF) {
        ASTNode* stmt = NULL;
        if(p->curr->type==TOK_KW_FUNC) {
            eat(p,TOK_KW_FUNC); char* n=strdup(p->curr->value); eat(p,TOK_ID); eat(p,TOK_LPAREN);
            ASTNode *params=NULL, *pc=NULL;
            while(p->curr->type!=TOK_RPAREN) {
                ASTNode* pa = mk_node(NODE_ID,0,0,0,p->curr->value); eat(p,TOK_ID);
                if(!params) params=pa; else pc->right=pa; pc=pa;
                if(p->curr->type==TOK_COMMA) eat(p,TOK_COMMA);
            }
            eat(p,TOK_RPAREN);
            stmt = mk_node(NODE_FUNC_DEF, params, NULL, parse_block(p), n);
        } else if(p->curr->type==TOK_KW_IMPORT) {
            stmt = parse_import(p);
        } else {
            if(p->curr->type==TOK_KW_SHOW) { eat(p,TOK_KW_SHOW); eat(p,TOK_LPAREN); stmt=mk_node(NODE_SHOW, parse_expr(p),0,0,0); eat(p,TOK_RPAREN); }
            else stmt = parse_expr(p);
        }
        ASTNode* n = mk_node(NODE_PROG, stmt, NULL, NULL, NULL);
        if(!head) head=n; else curr->right=n; curr=n;
        if(p->curr->type==TOK_SEMICOLON) eat(p,TOK_SEMICOLON);
    }
    return head;
}

// VM

typedef enum { VAL_NUM, VAL_STR, VAL_FUNC } ValType;
typedef struct { ValType type; union { int n; char* s; ASTNode* f; } as; } Value;
typedef struct { char* name; Value val; } Var;
typedef struct { Var* vars; int cnt; int cap; } Scope;
typedef struct { Scope* scopes; int sc_cnt; bool is_ret; Value ret_val; } VM;

Value mk_num(int n) { Value v={VAL_NUM, .as.n=n}; return v; }
Value mk_str(char* s) { Value v={VAL_STR, .as.s=strdup(s)}; return v; }

VM* create_vm() {
    VM* vm = malloc(sizeof(VM)); vm->scopes=malloc(sizeof(Scope)*100);
    vm->scopes[0].vars=malloc(sizeof(Var)*100); vm->scopes[0].cnt=0; vm->scopes[0].cap=100;
    vm->sc_cnt=1; vm->is_ret=false;
    return vm;
}

void set_var(VM* vm, char* n, Value v) {
    Scope* s = &vm->scopes[vm->sc_cnt-1];
    for(int i=0; i<s->cnt; i++) if(!strcmp(s->vars[i].name, n)) { s->vars[i].val=v; return; }
    if(s->cnt >= s->cap) { s->cap*=2; s->vars=realloc(s->vars, sizeof(Var)*s->cap); }
    s->vars[s->cnt].name=strdup(n); s->vars[s->cnt].val=v; s->cnt++;
}

void set_global(VM* vm, char* n, Value v) {
    Scope* s = &vm->scopes[0];
    for(int i=0; i<s->cnt; i++) if(!strcmp(s->vars[i].name, n)) { s->vars[i].val=v; return; }
    if(s->cnt >= s->cap) { s->cap*=2; s->vars=realloc(s->vars, sizeof(Var)*s->cap); }
    s->vars[s->cnt].name=strdup(n); s->vars[s->cnt].val=v; s->cnt++;
}

Value get_var(VM* vm, char* n) {
    for(int k=vm->sc_cnt-1; k>=0; k--) 
        for(int i=0; i<vm->scopes[k].cnt; i++) 
            if(!strcmp(vm->scopes[k].vars[i].name, n)) {
                Value v=vm->scopes[k].vars[i].val;
                return v.type==VAL_STR ? mk_str(v.as.s) : v;
            }
    printf("Runtime Error: Variable '%s' not found\n", n); exit(1);
}

char* read_file(const char* f) {
    FILE* file = fopen(f, "rb"); if(!file) return NULL;
    fseek(file,0,SEEK_END); long len=ftell(file); fseek(file,0,SEEK_SET);
    char* s=malloc(len+1); fread(s,1,len,file); s[len]=0; fclose(file);
    return s;
}

void run_block(void* vm_ptr, ASTNode* n);
ASTNode* parse_prog(Parser* p);

void exec_import(VM* vm, char* path) {
    char* src = read_file(path);
    if(!src) { printf("Error: Could not import '%s'\n", path); exit(1); }
    Lexer* l = create_lexer(src);
    Parser* p = create_parser(l);
    ASTNode* prog = parse_prog(p);
    run_block(vm, prog);
    free(src);
}

Value eval(VM* vm, ASTNode* n) {
    if(!n) return mk_num(0);
    if(n->type==NODE_NUM) return mk_num(atoi(n->value));
    if(n->type==NODE_STR) return mk_str(n->value);
    if(n->type==NODE_ID) return get_var(vm, n->value);
    
    if(n->type==NODE_IMPORT) { exec_import(vm, n->value); return mk_num(0); }
    if(n->type==NODE_ASSIGN) { Value v=eval(vm, n->right); set_var(vm, n->value, v); return v; }
    if(n->type==NODE_FUNC_DEF) { Value v={VAL_FUNC, .as.f=n}; set_global(vm, n->value, v); return mk_num(0); }
    
    if(n->type==NODE_IF) {
        Value c = eval(vm, n->left);
        if(c.as.n) run_block(vm, n->right);
        else if(n->extra) run_block(vm, n->extra);
        return mk_num(0);
    }
    if(n->type==NODE_WHILE) {
        while(eval(vm, n->left).as.n) {
            run_block(vm, n->right);
            if(vm->is_ret) break;
        }
        return mk_num(0);
    }
    if(n->type==NODE_FOR) {
        eval(vm, n->left);
        while(eval(vm, n->right).as.n) {
            run_block(vm, n->extra->right); // body
            if(vm->is_ret) break;
            eval(vm, n->extra->left); // step
        }
        return mk_num(0);
    }
    if(n->type==NODE_RETURN) {
        vm->is_ret = true;
        vm->ret_val = n->left ? eval(vm, n->left) : mk_num(0);
        return vm->ret_val;
    }
    if(n->type==NODE_FUNC_CALL) {
        Value f = get_var(vm, n->value);
        Value args[20]; int ac=0; ASTNode* a=n->left;
        while(a) { args[ac++]=eval(vm, a->left); a=a->right; }
        
        vm->scopes[vm->sc_cnt].vars=malloc(sizeof(Var)*20); 
        vm->scopes[vm->sc_cnt].cnt=0; vm->scopes[vm->sc_cnt].cap=20;
        vm->sc_cnt++;
        
        ASTNode* p = f.as.f->left; int i=0;
        while(p && i<ac) { set_var(vm, p->value, args[i++]); p=p->right; }
        
        run_block(vm, f.as.f->extra);
        
        Value ret = mk_num(0);
        if (vm->is_ret) { ret = vm->ret_val; vm->is_ret = false; }
        vm->sc_cnt--;
        return ret;
    }
    if(n->type==NODE_BIN_OP) {
        Value l=eval(vm,n->left), r=eval(vm,n->right);
        int res = 0;
        if(!strcmp(n->value,"+")) res = l.as.n+r.as.n;
        else if(!strcmp(n->value,"-")) res = l.as.n-r.as.n;
        else if(!strcmp(n->value,"*")) res = l.as.n*r.as.n;
        else if(!strcmp(n->value,"/")) res = l.as.n/r.as.n;
        else if(!strcmp(n->value,"<")) res = l.as.n < r.as.n;
        else if(!strcmp(n->value,">")) res = l.as.n > r.as.n;
        else if(!strcmp(n->value,"==")) res = l.as.n == r.as.n;
        else if(!strcmp(n->value,"!=")) res = l.as.n != r.as.n;
        return mk_num(res);
    }
    return mk_num(0);
}

void run_block(void* vm_ptr, ASTNode* n) {
    VM* vm = (VM*)vm_ptr;
    while(n) {
        if (vm->is_ret) return;
        if (n->left) {
            ASTNode* s = n->left;
            if(s->type==NODE_SHOW) {
                Value v=eval(vm, s->left);
                if(v.type==VAL_NUM) printf("%d\n", v.as.n); else printf("%s\n", v.as.s);
            } else {
                eval(vm, s);
            }
        }
        n = n->right;
    }
}

int main(int c, char** v) {
    if(c<2) { printf("Usage: ./main <file.df>\n"); return 1; }
    char* s=read_file(v[1]);
    if(!s) { printf("Could not open %s\n", v[1]); return 1; }
    run_block(create_vm(), parse_prog(create_parser(create_lexer(s))));
    return 0;
}
