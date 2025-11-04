#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>

#define func void

typedef struct {
    int id;
    char name[30];
} Object;

typedef struct {
    Object *items;
    int size;
    int capacity;
} ObjectList;

typedef uint8_t Byte;

typedef struct {
    char name[50];
    union {
        Byte byte_value;
        int int_value;
        float float_value;
        char str_value[100];
    } value;
    char type[20];
} Variable;

typedef struct {
    char name[50];
    char params[100];
    char body[500];
} ClassMethod;

typedef struct {
    char name[50];
    ClassMethod methods[20];
    int method_count;
    Variable attributes[20];
    int attribute_count;
} Class;

typedef struct {
    char name[50];
    char params[100];
    char body[500];
} Function;

Variable variables[100];
int variable_count = 0;

Class classes[20];
int class_count = 0;

Function functions[20];
int function_count = 0;

ObjectList* object_lists[20];
int object_list_count = 0;

ObjectList* create_object_list(int capacity) {
    ObjectList *list = malloc(sizeof(ObjectList));
    list->capacity = capacity;
    list->size = 0;
    list->items = malloc(sizeof(Object) * capacity);
    return list;
}

void add_object(ObjectList *list, int id, const char *name) {
    if (list->size >= list->capacity) {
        list->capacity *= 2;
        list->items = realloc(list->items, sizeof(Object) * list->capacity);
    }
    Object obj;
    obj.id = id;
    strncpy(obj.name, name, sizeof(obj.name) - 1);
    obj.name[sizeof(obj.name) - 1] = '\0';
    list->items[list->size] = obj;
    list->size++;
}

void clear_object_list(ObjectList *list) {
    free(list->items);
    free(list);
}

void show(const char *text) {
    printf("%s\n", text);
}

char* get_inp(const char *message, const char *prompt) {
    printf("%s\n", message);
    printf("%s", prompt);
    fflush(stdout);
    
    char *buffer = malloc(100 * sizeof(char));
    if (buffer == NULL) {
        perror("Memory allocation failed");
        return NULL;
    }

    if (fgets(buffer, 100, stdin) == NULL) {
        free(buffer);
        return NULL;
    }

    buffer[strcspn(buffer, "\n")] = '\0';
    return buffer;
}

Byte get_byte(const char *value) {
    int int_value = atoi(value);
    return (Byte)(int_value & 0xFF);
}

void set_variable(const char *name, int value, const char *type) {
    for (int i = 0; i < variable_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            if (strcmp(type, "byte") == 0) {
                variables[i].value.byte_value = (Byte)value;
            } else if (strcmp(type, "int") == 0) {
                variables[i].value.int_value = value;
            }
            return;
        }
    }

    if (variable_count < 100) {
        strncpy(variables[variable_count].name, name, sizeof(variables[variable_count].name) - 1);
        variables[variable_count].name[sizeof(variables[variable_count].name) - 1] = '\0';

        if (strcmp(type, "byte") == 0) {
            variables[variable_count].value.byte_value = (Byte)value;
            strncpy(variables[variable_count].type, "byte", sizeof(variables[variable_count].type) - 1);
        } else if (strcmp(type, "int") == 0) {
            variables[variable_count].value.int_value = value;
            strncpy(variables[variable_count].type, "int", sizeof(variables[variable_count].type) - 1);
        }
        variable_count++;
    }
}

void set_variable_string(const char *name, const char *value) {
    for (int i = 0; i < variable_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            strncpy(variables[i].value.str_value, value, sizeof(variables[i].value.str_value) - 1);
            variables[i].value.str_value[sizeof(variables[i].value.str_value) - 1] = '\0';
            strncpy(variables[i].type, "string", sizeof(variables[i].type) - 1);
            return;
        }
    }

    if (variable_count < 100) {
        strncpy(variables[variable_count].name, name, sizeof(variables[variable_count].name) - 1);
        variables[variable_count].name[sizeof(variables[variable_count].name) - 1] = '\0';
        
        strncpy(variables[variable_count].value.str_value, value, sizeof(variables[variable_count].value.str_value) - 1);
        variables[variable_count].value.str_value[sizeof(variables[variable_count].value.str_value) - 1] = '\0';
        strncpy(variables[variable_count].type, "string", sizeof(variables[variable_count].type) - 1);
        
        variable_count++;
    }
}

int get_variable_int(const char *name) {
    for (int i = 0; i < variable_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            if (strcmp(variables[i].type, "byte") == 0) {
                return variables[i].value.byte_value;
            } else if (strcmp(variables[i].type, "int") == 0) {
                return variables[i].value.int_value;
            }
        }
    }
    return 0;
}

const char* get_variable_string(const char *name) {
    for (int i = 0; i < variable_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            if (strcmp(variables[i].type, "string") == 0) {
                return variables[i].value.str_value;
            } else if (strcmp(variables[i].type, "int") == 0) {
                static char buffer[20];
                snprintf(buffer, sizeof(buffer), "%d", variables[i].value.int_value);
                return buffer;
            }
        }
    }
    return "";
}

void add_class(const char *name) {
    if (class_count < 20) {
        strncpy(classes[class_count].name, name, sizeof(classes[class_count].name) - 1);
        classes[class_count].name[sizeof(classes[class_count].name) - 1] = '\0';
        classes[class_count].method_count = 0;
        classes[class_count].attribute_count = 0;
        class_count++;
    }
}

void add_class_method(const char *class_name, const char *method_name, const char *method_params, const char *method_body) {
    for (int i = 0; i < class_count; i++) {
        if (strcmp(classes[i].name, class_name) == 0) {
            if (classes[i].method_count < 20) {
                strncpy(classes[i].methods[classes[i].method_count].name, method_name,
                        sizeof(classes[i].methods[classes[i].method_count].name) - 1);
                classes[i].methods[classes[i].method_count].name[sizeof(classes[i].methods[classes[i].method_count].name) - 1] = '\0';

                if (method_params) {
                    strncpy(classes[i].methods[classes[i].method_count].params, method_params,
                            sizeof(classes[i].methods[classes[i].method_count].params) - 1);
                    classes[i].methods[classes[i].method_count].params[sizeof(classes[i].methods[classes[i].method_count].params) - 1] = '\0';
                }

                strncpy(classes[i].methods[classes[i].method_count].body, method_body,
                        sizeof(classes[i].methods[classes[i].method_count].body) - 1);
                classes[i].methods[classes[i].method_count].body[sizeof(classes[i].methods[classes[i].method_count].body) - 1] = '\0';

                classes[i].method_count++;
            }
            return;
        }
    }
}

void add_function(const char *func_name, const char *func_params, const char *func_body) {
    if (function_count < 20) {
        strncpy(functions[function_count].name, func_name, sizeof(functions[function_count].name) - 1);
        functions[function_count].name[sizeof(functions[function_count].name) - 1] = '\0';

        if (func_params && strlen(func_params) > 0) {
            strncpy(functions[function_count].params, func_params, sizeof(functions[function_count].params) - 1);
            functions[function_count].params[sizeof(functions[function_count].params) - 1] = '\0';
        } else {
            functions[function_count].params[0] = '\0';
        }

        if (func_body && strlen(func_body) > 0) {
            strncpy(functions[function_count].body, func_body, sizeof(functions[function_count].body) - 1);
            functions[function_count].body[sizeof(functions[function_count].body) - 1] = '\0';
        } else {
            functions[function_count].body[0] = '\0';
        }

        function_count++;
    }
}

bool evaluate_condition(const char *condition) {
    char *cond_copy = strdup(condition);
    if (cond_copy == NULL) return false;

    bool result = false;

    if (strstr(cond_copy, "==") != NULL) {
        char *left = strtok(cond_copy, "==");
        char *right = strtok(NULL, "==");

        if (left != NULL && right != NULL) {
            left = strtok(left, " \t");
            right = strtok(right, " \t");

            if (left != NULL && right != NULL) {
                if (right[0] == '"' && right[strlen(right)-1] == '"') {
                    right++;
                    right[strlen(right)-1] = '\0';
                    
                    char left_value[100] = "";
                    for (int i = 0; i < variable_count; i++) {
                        if (strcmp(variables[i].name, left) == 0) {
                            if (strcmp(variables[i].type, "string") == 0) {
                                strncpy(left_value, variables[i].value.str_value, sizeof(left_value)-1);
                            } else if (strcmp(variables[i].type, "int") == 0) {
                                snprintf(left_value, sizeof(left_value), "%d", variables[i].value.int_value);
                            }
                            break;
                        }
                    }
                    
                    if (strcmp(left_value, right) == 0) {
                        result = true;
                    }
                }
                else {
                    int left_val = get_variable_int(left);
                    int right_val = atoi(right);
                    if (left_val == right_val) {
                        result = true;
                    }
                }
            }
        }
    }
    else if (strstr(cond_copy, "!=") != NULL) {
        char *left = strtok(cond_copy, "!=");
        char *right = strtok(NULL, "!=");

        if (left != NULL && right != NULL) {
            left = strtok(left, " \t");
            right = strtok(right, " \t");

            if (left != NULL && right != NULL) {
                int left_val = get_variable_int(left);
                int right_val = atoi(right);
                if (left_val != right_val) {
                    result = true;
                }
            }
        }
    }

    free(cond_copy);
    return result;
}

int evaluate_expression(const char *expr) {
    char expr_copy[100];
    strncpy(expr_copy, expr, sizeof(expr_copy) - 1);
    expr_copy[sizeof(expr_copy) - 1] = '\0';

    char *token;
    int result = 0;
    char current_operator = '+';
    int current_value;
    
    char clean_expr[100] = "";
    char *src = expr_copy;
    char *dst = clean_expr;
    while (*src) {
        if (*src != ' ' && *src != '\t') {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';

    token = strtok(clean_expr, "+-*/");
    while (token != NULL) {
        char *operator_pos = clean_expr + (token - clean_expr) - 1;
        if (operator_pos >= clean_expr) {
            if (*operator_pos == '+') current_operator = '+';
            else if (*operator_pos == '-') current_operator = '-';
            else if (*operator_pos == '*') current_operator = '*';
            else if (*operator_pos == '/') current_operator = '/';
        }

        if (isdigit(token[0]) || (token[0] == '-' && isdigit(token[1]))) {
            current_value = atoi(token);
        } else {
            current_value = get_variable_int(token);
        }

        switch (current_operator) {
            case '+': result += current_value; break;
            case '-': result -= current_value; break;
            case '*': result *= current_value; break;
            case '/': 
                if (current_value != 0) result /= current_value; 
                break;
        }

        token = strtok(NULL, "+-*/");
    }
    
    return result;
}

char* parse_show_expression(const char *content) {
    char *result = malloc(256);
    if (!result) return NULL;
    result[0] = '\0';

    char content_copy[256];
    strncpy(content_copy, content, sizeof(content_copy) - 1);
    content_copy[sizeof(content_copy) - 1] = '\0';

    char *pos = content_copy;
    char temp[256] = "";
    int temp_index = 0;

    while (*pos) {
        if (*pos == '"') {
            pos++;
            while (*pos && *pos != '"') {
                temp[temp_index++] = *pos++;
            }
            if (*pos == '"') pos++;
            temp[temp_index] = '\0';
            strcat(result, temp);
            temp_index = 0;
            temp[0] = '\0';
        }
        else if (*pos == '+' && (pos == content_copy || *(pos-1) != 'e' && *(pos-1) != 'E')) {
            pos++;
        }
        else if (*pos == '(') {
            pos++;
            char expr[100] = "";
            int expr_index = 0;
            int paren_count = 1;

            while (*pos && paren_count > 0) {
                if (*pos == '(') paren_count++;
                else if (*pos == ')') paren_count--;
                
                if (paren_count > 0) {
                    expr[expr_index++] = *pos;
                }
                pos++;
            }
            expr[expr_index] = '\0';
            int math_result = evaluate_expression(expr);
            char num_str[20];
            snprintf(num_str, sizeof(num_str), "%d", math_result);
            strcat(result, num_str);
        }
        else {
            temp[temp_index++] = *pos++;
            temp[temp_index] = '\0';

            if (!*pos || *pos == '+' || *pos == '"') {
                if (strchr(temp, '+') != NULL || strchr(temp, '-') != NULL || 
                    strchr(temp, '*') != NULL || strchr(temp, '/') != NULL) {
                    int math_result = evaluate_expression(temp);
                    char num_str[20];
                    snprintf(num_str, sizeof(num_str), "%d", math_result);
                    strcat(result, num_str);
                } else {
                    const char *var_value = get_variable_string(temp);
                    if (strlen(var_value) > 0) {
                        strcat(result, var_value);
                    } else {
                        strcat(result, temp);
                    }
                }
                temp_index = 0;
                temp[0] = '\0';
            }
        }
    }

    if (temp_index > 0) {
        temp[temp_index] = '\0';
        if (strchr(temp, '+') != NULL || strchr(temp, '-') != NULL || 
            strchr(temp, '*') != NULL || strchr(temp, '/') != NULL) {
            int math_result = evaluate_expression(temp);
            char num_str[20];
            snprintf(num_str, sizeof(num_str), "%d", math_result);
            strcat(result, num_str);
        } else {
            const char *var_value = get_variable_string(temp);
            if (strlen(var_value) > 0) {
                strcat(result, var_value);
            } else {
                strcat(result, temp);
            }
        }
    }

    return result;
}

void import_from_file(const char *filename, const char *elements) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening imported file");
        return;
    }

    char line[256];
    bool in_imported_block = false;

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;

        if (strlen(line) == 0 || line[0] == '#') {
            continue;
        }

        if (strstr(line, "func ") != NULL) {
            char *func_name_start = strstr(line, "func ") + 5;
            char *func_name_end = strchr(func_name_start, '(');
            if (func_name_end != NULL) {
                *func_name_end = '\0';

                if (strstr(elements, func_name_start) != NULL) {
                    in_imported_block = true;
                }
            }
        }

        if (in_imported_block) {
            if (strstr(line, "}") != NULL) {
                in_imported_block = false;
            }
        }
    }

    fclose(file);
}

void create_file(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error creating file");
        return;
    }
    fclose(file);
}

void rm_file(const char *filename) {
    if (remove(filename) != 0) {
        perror("Error deleting file");
    }
}

void clear_file_content(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file for clearing");
        return;
    }
    fclose(file);
}

void put_content_in_file(const char *content, const char *filename) {
    FILE *file = fopen(filename, "a");
    if (file == NULL) {
        perror("Error opening file for writing");
        return;
    }

    fprintf(file, "%s\n", content);
    fclose(file);
}

char* read_file_content(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file for reading");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *content = malloc(file_size + 1);
    if (content == NULL) {
        perror("Memory allocation failed");
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(content, 1, file_size, file);
    if (bytes_read < file_size) {
        perror("Error reading file");
        free(content);
        fclose(file);
        return NULL;
    }

    content[bytes_read] = '\0';
    fclose(file);
    return content;
}

void process_for_loop(const char *loop_line, FILE *file) {
    char *var_start = strstr(loop_line, "for (") + 5;
    char *var_end = strchr(var_start, ' ');
    if (var_end == NULL) return;

    *var_end = '\0';
    char loop_var[50];
    strncpy(loop_var, var_start, sizeof(loop_var) - 1);
    loop_var[sizeof(loop_var) - 1] = '\0';

    if (strstr(var_end + 1, "in range[") != NULL) {
        char *range_start = strstr(var_end + 1, "in range[") + 9;
        char *range_end = strchr(range_start, ']');
        if (range_end != NULL) {
            *range_end = '\0';
            int range_value = atoi(range_start);

            char loop_body[500] = "";
            char line[256];
            bool in_loop_body = true;

            while (in_loop_body && fgets(line, sizeof(line), file)) {
                line[strcspn(line, "\n")] = 0;

                if (strstr(line, "}") != NULL) {
                    in_loop_body = false;
                    break;
                }

                if (strlen(loop_body) + strlen(line) + 2 < sizeof(loop_body)) {
                    strcat(loop_body, line);
                    strcat(loop_body, "\n");
                }
            }

            for (int i = 0; i < range_value; i++) {
                set_variable(loop_var, i, "int");
            }
        }
    }
}

void process_simple_assignment(const char *line) {
    char *equals_pos = strchr(line, '=');
    if (equals_pos != NULL) {
        *equals_pos = '\0';
        char *var_name = line;
        char *value_str = equals_pos + 1;
        
        while (*var_name == ' ' || *var_name == '\t') var_name++;
        char *var_end = var_name + strlen(var_name) - 1;
        while (var_end > var_name && (*var_end == ' ' || *var_end == '\t')) var_end--;
        *(var_end + 1) = '\0';
        
        while (*value_str == ' ' || *value_str == '\t') value_str++;
        char *value_end = value_str + strlen(value_str) - 1;
        while (value_end > value_str && (*value_end == ' ' || *value_end == '\t')) value_end--;
        *(value_end + 1) = '\0';
        
        if (strlen(value_str) > 0) {
            if (strchr(value_str, '+') != NULL || strchr(value_str, '-') != NULL || 
                strchr(value_str, '*') != NULL || strchr(value_str, '/') != NULL) {
                int result = evaluate_expression(value_str);
                set_variable(var_name, result, "int");
            } else {
                bool is_number = true;
                for (char *p = value_str; *p; p++) {
                    if (!isdigit(*p) && *p != '-') {
                        is_number = false;
                        break;
                    }
                }
                
                if (is_number) {
                    set_variable(var_name, atoi(value_str), "int");
                } else {
                    set_variable_string(var_name, value_str);
                }
            }
        }
    }
}

void execute_function_body(const char *body, const char *params, const char *args) {
    char body_copy[500];
    strncpy(body_copy, body, sizeof(body_copy) - 1);
    body_copy[sizeof(body_copy) - 1] = '\0';

    Variable saved_variables[100];
    int saved_variable_count = variable_count;
    memcpy(saved_variables, variables, sizeof(variables));

    if (params && args && strlen(params) > 0 && strlen(args) > 0) {
        char params_copy[100], args_copy[100];
        strncpy(params_copy, params, sizeof(params_copy) - 1);
        strncpy(args_copy, args, sizeof(args_copy) - 1);
        params_copy[sizeof(params_copy) - 1] = '\0';
        args_copy[sizeof(args_copy) - 1] = '\0';
        
        char *param_ptr = params_copy;
        char *arg_ptr = args_copy;
        
        while (*param_ptr && *arg_ptr) {
            char param_name[50] = "";
            char *param_start = param_ptr;
            while (*param_ptr && *param_ptr != ',') param_ptr++;
            strncpy(param_name, param_start, param_ptr - param_start);
            if (*param_ptr == ',') param_ptr++;
            
            char arg_value[50] = "";
            char *arg_start = arg_ptr;
            while (*arg_ptr && *arg_ptr != ',') arg_ptr++;
            strncpy(arg_value, arg_start, arg_ptr - arg_start);
            if (*arg_ptr == ',') arg_ptr++;
            
            char *trimmed_param = param_name;
            while (*trimmed_param == ' ' || *trimmed_param == '\t') trimmed_param++;
            char *param_end = trimmed_param + strlen(trimmed_param) - 1;
            while (param_end > trimmed_param && (*param_end == ' ' || *param_end == '\t')) param_end--;
            *(param_end + 1) = '\0';
            
            char *trimmed_arg = arg_value;
            while (*trimmed_arg == ' ' || *trimmed_arg == '\t') trimmed_arg++;
            char *arg_end = trimmed_arg + strlen(trimmed_arg) - 1;
            while (arg_end > trimmed_arg && (*arg_end == ' ' || *arg_end == '\t')) arg_end--;
            *(arg_end + 1) = '\0';
            
            if (trimmed_arg[0] == '"' && trimmed_arg[strlen(trimmed_arg)-1] == '"') {
                memmove(trimmed_arg, trimmed_arg + 1, strlen(trimmed_arg) - 2);
                trimmed_arg[strlen(trimmed_arg) - 2] = '\0';
                set_variable_string(trimmed_param, trimmed_arg);
            } else {
                if (strchr(trimmed_arg, '+') != NULL || strchr(trimmed_arg, '-') != NULL || 
                    strchr(trimmed_arg, '*') != NULL || strchr(trimmed_arg, '/') != NULL) {
                    int result = evaluate_expression(trimmed_arg);
                    set_variable(trimmed_param, result, "int");
                } else {
                    bool is_number = true;
                    for (char *p = trimmed_arg; *p; p++) {
                        if (!isdigit(*p) && *p != '-') {
                            is_number = false;
                            break;
                        }
                    }
                    
                    if (is_number) {
                        set_variable(trimmed_param, atoi(trimmed_arg), "int");
                    } else {
                        int value = get_variable_int(trimmed_arg);
                        set_variable(trimmed_param, value, "int");
                    }
                }
            }
            
            if (!*param_ptr || !*arg_ptr) break;
        }
    }

    char *body_line = strtok(body_copy, "\n");
    while (body_line != NULL) {
        char temp_line[256];
        strncpy(temp_line, body_line, sizeof(temp_line) - 1);
        temp_line[sizeof(temp_line) - 1] = '\0';

        char *line_start = temp_line;
        while (*line_start == ' ' || *line_start == '\t') line_start++;

        if (strchr(line_start, '=') != NULL && 
            strstr(line_start, "get_inp") == NULL && 
            strstr(line_start, "get_byte") == NULL && 
            strstr(line_start, "read_file_content") == NULL) {
            
            process_simple_assignment(line_start);
        }
        else if (strstr(line_start, "show(") != NULL) {
            char *start = strchr(line_start, '(');
            if (start != NULL) {
                start++;
                
                int show_paren_count = 1;
                char *show_end = start;
                while (*show_end && show_paren_count > 0) {
                    if (*show_end == '(') show_paren_count++;
                    else if (*show_end == ')') show_paren_count--;
                    show_end++;
                }
                
                if (show_paren_count == 0) {
                    show_end--;
                    
                    char show_content[256];
                    int content_len = show_end - start;
                    strncpy(show_content, start, content_len);
                    show_content[content_len] = '\0';
                    
                    char *output = parse_show_expression(show_content);
                    if (output) {
                        show(output);
                        free(output);
                    }
                }
            }
        }

        body_line = strtok(NULL, "\n");
    }

    variable_count = saved_variable_count;
    memcpy(variables, saved_variables, sizeof(variables));
}

void interpret_df_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    char line[256];
    bool in_if_block = false;
    bool if_condition_met = false;
    bool in_loop_block = false;
    char loop_condition[256] = "";
    bool in_class_block = false;
    char current_class[50] = "";
    bool in_method_block = false;
    char current_method[50] = "";
    char method_params[100] = "";
    char method_body[500] = "";

    variable_count = 0;
    class_count = 0;
    function_count = 0;
    object_list_count = 0;

    fseek(file, 0, SEEK_SET);
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;

        if (strlen(line) == 0 || line[0] == '#') {
            continue;
        }

        if (strstr(line, "func ") == line && strchr(line, '(') != NULL && strchr(line, ')') != NULL && 
            !in_class_block) {
            
            char *func_start = strstr(line, "func ") + 5;
            
            char *open_paren = strchr(func_start, '(');
            if (open_paren != NULL) {
                int name_length = open_paren - func_start;
                char func_name[50] = "";
                strncpy(func_name, func_start, name_length);
                func_name[name_length] = '\0';
                
                char *trimmed_name = func_name;
                while (*trimmed_name == ' ' || *trimmed_name == '\t') trimmed_name++;
                char *name_end = trimmed_name + strlen(trimmed_name) - 1;
                while (name_end > trimmed_name && (*name_end == ' ' || *name_end == '\t')) name_end--;
                *(name_end + 1) = '\0';
                
                char *close_paren = strchr(open_paren, ')');
                if (close_paren != NULL) {
                    int params_length = close_paren - open_paren - 1;
                    char func_params[100] = "";
                    if (params_length > 0) {
                        strncpy(func_params, open_paren + 1, params_length);
                        func_params[params_length] = '\0';
                    }
                    
                    char func_body[500] = "";
                    bool in_func_body = true;
                    int brace_count = 1;
                    
                    while (in_func_body && fgets(line, sizeof(line), file)) {
                        line[strcspn(line, "\n")] = 0;

                        char *brace_pos = line;
                        while ((brace_pos = strchr(brace_pos, '{')) != NULL) {
                            brace_count++;
                            brace_pos++;
                        }
                        
                        brace_pos = line;
                        while ((brace_pos = strchr(brace_pos, '}')) != NULL) {
                            brace_count--;
                            brace_pos++;
                            if (brace_count == 0) {
                                in_func_body = false;
                                break;
                            }
                        }

                        if (in_func_body) {
                            if (strlen(func_body) + strlen(line) + 2 < sizeof(func_body)) {
                                strcat(func_body, line);
                                strcat(func_body, "\n");
                            }
                        }
                    }

                    add_function(trimmed_name, func_params, func_body);
                }
            }
        }
    }

    fseek(file, 0, SEEK_SET);
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;

        if (strlen(line) == 0 || line[0] == '#') {
            continue;
        }

        if ((strstr(line, "func ") == line && strchr(line, '(') != NULL && strchr(line, ')') != NULL) ||
            (strstr(line, "class ") != NULL && strchr(line, '(') != NULL && strchr(line, ')') != NULL)) {
            while (fgets(line, sizeof(line), file)) {
                line[strcspn(line, "\n")] = 0;
                if (strstr(line, "}") != NULL) {
                    break;
                }
            }
            continue;
        }

        if (strchr(line, '(') != NULL && strchr(line, ')') != NULL && 
            strstr(line, "func ") != line) {
            
            char *open_paren = strchr(line, '(');
            if (open_paren != NULL) {
                int name_length = open_paren - line;
                char func_name[50] = "";
                strncpy(func_name, line, name_length);
                func_name[name_length] = '\0';
                
                char *trimmed_name = func_name;
                while (*trimmed_name == ' ' || *trimmed_name == '\t') trimmed_name++;
                char *name_end = trimmed_name + strlen(trimmed_name) - 1;
                while (name_end > trimmed_name && (*name_end == ' ' || *name_end == '\t')) name_end--;
                *(name_end + 1) = '\0';
                
                char *close_paren = strchr(open_paren, ')');
                if (close_paren != NULL) {
                    int args_length = close_paren - open_paren - 1;
                    char func_args[100] = "";
                    if (args_length > 0) {
                        strncpy(func_args, open_paren + 1, args_length);
                        func_args[args_length] = '\0';
                    }

                    if (strcmp(trimmed_name, "show") == 0) {
                        char *output = parse_show_expression(func_args);
                        if (output) {
                            show(output);
                            free(output);
                        }
                    }
                    else {
                        bool function_found = false;
                        for (int i = 0; i < function_count; i++) {
                            if (strcmp(functions[i].name, trimmed_name) == 0) {
                                execute_function_body(functions[i].body, functions[i].params, func_args);
                                function_found = true;
                                break;
                            }
                        }
                    }
                }
            }
            continue;
        }

        if (strchr(line, '=') != NULL && 
            strstr(line, "get_inp") == NULL && 
            strstr(line, "get_byte") == NULL && 
            strstr(line, "read_file_content") == NULL) {
            
            process_simple_assignment(line);
            continue;
        }
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <filename.df>\n", argv[0]);
        return 1;
    }

    interpret_df_file(argv[1]);

    for (int i = 0; i < object_list_count; i++) {
        clear_object_list(object_lists[i]);
    }

    variable_count = 0;
    class_count = 0;
    object_list_count = 0;
    function_count = 0;

    return 0;
}
