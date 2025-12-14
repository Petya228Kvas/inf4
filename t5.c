#include <stdio.h>
#include <string.h>
#include <ctype.h>
#define BUFFER_SIZE 1000      // Максимальный размер входного буфера
#define MAX_VARS 100          // Максимальное количество переменных
#define MAX_VAR_LEN 50        // Максимальная длина имени переменной

// Глобальные переменные для хранения состояния программы
char input[BUFFER_SIZE];      // Буфер для хранения всего входного текста
int input_len = 0;            // Фактическая длина входного текста

char variables[MAX_VARS][MAX_VAR_LEN];  // Массив для хранения имен объявленных переменных
int var_count = 0;                      // Количество объявленных переменных

int in_condition_block = 0;   // Флаг: находимся ли внутри блока условия (if)
int condition_indent = -1;    // Уровень отступа для текущего условия (-1 = нет активного условия)

void add_variable(const char* name);     // Добавление переменной в список
int is_variable(const char* name);       // Проверка, объявлена ли переменная
void skip_spaces(int* pos);              // Пропуск пробелов в позиции
int get_indentation(int pos);            // Получение уровня отступа строки
int get_identifier(int* pos, char* buf); // Извлечение идентификатора (имени переменной)
int is_string_literal(const char* str);  // Проверка, является ли строка строковым литералом

int main() {
    int c;
    while ((c = getchar()) != EOF && input_len < BUFFER_SIZE - 1) {
        input[input_len++] = c;
    }
    input[input_len] = '\0';
    
    int line_start = 0;
    
    // Обработка входного текста построчно
    for (int i = 0; i <= input_len; i++) {
        if (i == input_len || input[i] == '\n') {
            int line_end = i;
            int line_indent = get_indentation(line_start);
            int pos = line_start + line_indent;
            if (condition_indent != -1 && line_indent <= condition_indent) {
                for (int j = 0; j < condition_indent; j++) {
                    putchar(' ');
                }
                printf("}\n"); 
                condition_indent = -1;
                in_condition_block = 0;
            }
            for (int j = 0; j < line_indent; j++) {
                putchar(' ');
            }
            
            // Если строка не пустая (не состоит только из отступов)
            if (pos < line_end) {
                char var_name[MAX_VAR_LEN];  // Буфер для имени переменной
                
                // Обработка оператора IF
                if (strncmp(&input[pos], "if ", 3) == 0 || 
                    (pos + 2 < line_end && strncmp(&input[pos], "if", 2) == 0 && isspace(input[pos+2]))) {
                    
                    // Преобразуем Python: if x > 0:  в C: if (x > 0) {
                    printf("if (");
                    pos += 2; 
                    skip_spaces(&pos); 
                    
                    // Копируем условие до двоеточия
                    while (pos < line_end && input[pos] != ':') {
                        if (input[pos] == '#') break;
                        putchar(input[pos++]);
                    }
                    printf(") {\n");
                    
                    condition_indent = line_indent; 
                    in_condition_block = 1;        
                }
                // Обработка оператора PRINT
                else if (strncmp(&input[pos], "print(", 6) == 0) {
                    // Преобразуем Python: print(...) в C: printf(...)
                    pos += 6; 
                    skip_spaces(&pos); 
                    printf("printf(\"");
                    
                    // Определяем тип аргумента для выбора правильного спецификатора формата
                    if (pos < line_end && (input[pos] == '"' || input[pos] == '\'')) {
                        printf("%%s\", ");
                        while (pos < line_end && input[pos] != ')' && input[pos] != '\n') {
                            if (input[pos] == '#') break;
                            putchar(input[pos++]);
                        }
                    } else {
                        char identifier[MAX_VAR_LEN];
                        if (get_identifier(&pos, identifier)) {
                            if (is_variable(identifier)) {
                                // Если переменная объявлена ранее - предполагаем int, используем %d
                                printf("%%d\", %s", identifier);
                            } else {
                                // Если не объявлена - предполагаем строку, используем %s
                                printf("%%s\", %s", identifier);
                            }
                        } else {
                            // Если не идентификатор, используем %s по умолчанию
                            printf("%%s\", ");
                            while (pos < line_end && input[pos] != ')' && input[pos] != '\n') {
                                if (input[pos] == '#') break;
                                putchar(input[pos++]);
                            }
                        }
                    }
                    printf(");\n"); 
                    
                    if (pos < line_end && input[pos] == ')') {
                        pos++;
                    }
                }
                // Обработка присваивания (x = ...)
                else if (get_identifier(&pos, var_name)) {
                    skip_spaces(&pos); 
                    
                    if (pos < line_end && input[pos] == '=') {
                        pos++;
                        skip_spaces(&pos); 
                        
                        // Обработка ввода с клавиатуры: x = int(input())
                        if (strncmp(&input[pos], "int(input())", 12) == 0) {
                            pos += 12;
                            // Определяем, нужно ли объявлять переменную
                            if (!in_condition_block && !is_variable(var_name)) {
                                // Если не в блоке условия и переменная не объявлена - объявляем
                                printf("int %s;\n", var_name);
                                add_variable(var_name);  // Добавляем в список переменных
                                for (int j = 0; j < line_indent; j++) putchar(' ');
                                printf("scanf(\"%%d\", &%s);\n", var_name);
                            } else {
                                printf("scanf(\"%%d\", &%s);\n", var_name);
                                if (!is_variable(var_name)) {
                                    add_variable(var_name);  // Добавляем в список
                                }
                            }
                        } else {
                            // Обработка обычного присваивания: x = 42 или x = a + 1
                            if (!in_condition_block && !is_variable(var_name)) {
                                // Если не в блоке условия и переменная не объявлена
                                printf("int %s = ", var_name);  // Объявляем с инициализацией
                                add_variable(var_name);
                            } else {
                                // Если в блоке условия или переменная уже объявлена
                                printf("%s = ", var_name);
                                if (!is_variable(var_name)) {
                                    add_variable(var_name);
                                }
                            }
                            
                            // Копируем выражение после '=' до конца строки
                            while (pos < line_end && input[pos] != '\n') {
                                if (input[pos] == '#') break; 
                                putchar(input[pos++]);
                            }
                            printf(";\n"); 
                        }
                    } else {
                        // Если нет оператора присваивания, просто копируем строку
                        while (pos < line_end) {
                            putchar(input[pos++]);
                        }
                        printf("\n");
                    }
                }
                // Обработка всех остальных строк (просто копируем)
                else {
                    while (pos < line_end) {
                        putchar(input[pos++]);
                    }
                    printf("\n");
                }
            } else {
                // Пустая строка (только отступы) - просто перенос строки
                printf("\n");
            }
            line_start = i + 1;
        }
    }
    
    // Если после обработки всех строк осталось незакрытое условие, закрываем его
    if (condition_indent != -1) {
        for (int j = 0; j < condition_indent; j++) {
            putchar(' ');
        }
        printf("}\n");
    }
    
    return 0;
}

// Добавляет переменную в список объявленных переменных
void add_variable(const char* name) {
    // Проверяем, не добавлена ли переменная уже
    for (int i = 0; i < var_count; i++) {
        if (strcmp(variables[i], name) == 0) {
            return;  // Переменная уже есть, не добавляем
        }
    }
    // Добавляем новую переменную, если есть место
    if (var_count < MAX_VARS) {
        strncpy(variables[var_count], name, MAX_VAR_LEN - 1);
        var_count++;
    }
}

// Проверяет, объявлена ли переменная с данным именем
int is_variable(const char* name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(variables[i], name) == 0) {
            return 1;  // Переменная найдена
        }
    }
    return 0;  // Переменная не найдена
}

// Пропускает пробельные символы (но не переносы строк)
void skip_spaces(int* pos) {
    while (*pos < input_len && isspace(input[*pos]) && input[*pos] != '\n') {
        (*pos)++;
    }
}

// Определяет уровень отступа строки (количество пробелов в начале)
int get_indentation(int pos) {
    int indent = 0;
    while (pos < input_len && input[pos] == ' ') {
        indent++;
        pos++;
    }
    return indent;
}

// Извлекает идентификатор (имя переменной) из текущей позиции
int get_identifier(int* pos, char* buf) {
    int i = 0;
    // Идентификатор должен начинаться с буквы или underscore
    if (*pos < input_len && (isalpha(input[*pos]) || input[*pos] == '_')) {
        buf[i++] = input[(*pos)++];
        // Продолжаем, пока идут буквы, цифры или underscore
        while (*pos < input_len && (isalnum(input[*pos]) || input[*pos] == '_')) {
            buf[i++] = input[(*pos)++];
        }
        buf[i] = '\0';
        return 1;    
    }
    return 0;  // Не удалось извлечь идентификатор
}

// Проверяет, является ли строка строковым литералом
int is_string_literal(const char* str) {
    return str[0] == '"' || str[0] == '\'';  // Начинается с кавычек
}
