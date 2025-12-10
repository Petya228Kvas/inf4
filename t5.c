#include <stdio.h>    
#include <stdbool.h> 

// --- Константы ---
#define MAX_INPUT 1000 // Максимальный размер входного буфера для Python-кода
#define MAX_BODY 500   // Максимальный размер буфера для тела блока if

// --- Глобальные переменные ---
char body_buf[MAX_BODY]; // Временный буфер для хранения сгенерированного C-кода тела блока if
int body_len;            // Текущая длина данных в body_buf

void put_str(const char *s);                        // Выводит строку посимвольно с помощью putchar
int skip_sp(const char *buf, int i);                // Пропускает пробелы, табы и переводы строк
int get_ind(const char *buf, int i, int *ind_chars); // Определяет количество символов отступа
int token(const char *buf, int i, char *token, int max_len); // Читает токен (ключевое слово/имя переменной)

int main() {
    char py[MAX_INPUT]; // Буфер для хранения всего входного Python-кода
    int char_count = 0; // Фактическое количество прочитанных символов
    int c;              // Для чтения символов через getchar()

    // 1 Чтение всего входного кода Python из stdin
    while ((c = getchar()) != EOF && char_count < MAX_INPUT - 1) {
        py[char_count++] = (char)c;
    }
    py[char_count] = '\0'; // Завершаем строку

    // 2 Вывод обязательной C-шапки
    put_str("#include <stdio.h>\n");
    put_str("\n");
    put_str("int main() {\n");
    
    // 3 Основной цикл парсинга
    int i = 0;             // Текущий индекс в буфере py
    char token_c[64];      // Буфер для хранения текущего токена (например if, print, x)
    
    while (i < char_count) {
        // Пропускаем все пробельные символы и пустые строки перед началом новой команды
        i = skip_sp(py, i);
        if (py[i] == '\0') {
            break; // Достигнут конец входного файла
        }
        
        int line_start = i; // Начало текущей логической команды
        
        // Читаем первый токен (ключевое слово или имя переменной)
        i = token(py, i, token_c, 64);
        int token_end_idx = i; 

        // Пропускаем пробелы после токена
        while (py[i] == ' ' || py[i] == '\t') {
            i++;
        }
        
        //Обработка условного оператора if 
        if (token_c[0] == 'i' && token_c[1] == 'f' && token_c[2] == '\0') {
            put_str("    if (");
            i = token_end_idx; // Начинаем чтение условия с конца if
            
            // Копируем условие (например, x > 0) до двоеточия
            while (py[i] != ':' && py[i] != '\0') {
                putchar(py[i]);
                i++;
            }
            put_str(") {\n"); // Открываем блок C
            
            // Пропускаем двоеточие и все пробельные символы до начала тела блока
            while (py[i] == ':' || py[i] == ' ' || py[i] == '\t' || py[i] == '\n' || py[i] == '\r') {
                i++;
            }
            
            int block_start_index = i;
            int init_ind; // Сюда будет записан размер отступа первой строки тела
            
            // Определяем начальный отступ для сравнения
            int body_start_index = get_ind(py, block_start_index, &init_ind);
            
            //Цикл сбора тела блока if
            body_len = 0;
            int current_line_start = body_start_index; // Индекс первого значащего символа в текущей строке
            
            while (i < char_count) {
                // 1 Находим конец текущей строки в Python-коде
                while (py[i] != '\n' && py[i] != '\r' && py[i] != '\0') {
                    i++;
                }
                
                // 2 Копируем тело команды в буфер body_buf
                // Добавляем отступ для C-кода (4 пробела)
                body_buf[body_len++] = ' '; body_buf[body_len++] = ' ';
                body_buf[body_len++] = ' '; body_buf[body_len++] = ' ';
                
                int j;
                for (j = current_line_start; j < i; j++) {
                    if (body_len < MAX_BODY - 2) {
                        body_buf[body_len++] = py[j];
                    }
                }
                body_buf[body_len++] = ';'; // Добавляем обязательную точку с запятой C
                body_buf[body_len++] = '\n';
                
                // 3 Пропускаем символы перевода строки до начала следующей
                while (py[i] == '\n' || py[i] == '\r') {
                    i++;
                }
                
                // 4 Проверяем отступ следующей строки для определения конца блока
                int next_ind;
                int start_of_next_token = get_ind(py, i, &next_ind); // Получаем отступ и сдвигаем i
                
                // Если новый отступ меньше начального, блок if закончился
                if (next_ind < init_ind && start_of_next_token < char_count) {
                    i = start_of_next_token; // Возвращаем индекс на начало следующей команды
                    break; 
                }
                
                // Иначе, это новая строка тела блока
                current_line_start = start_of_next_token; 
            }
            
            // Завершаем буфер и выводим сгенерированное тело блока
            body_buf[body_len] = '\0';
            put_str(body_buf);
            put_str("    }\n"); // Закрываем блок C
            continue;
        }


        //Обработка вывода (print)
        if (token_c[0] == 'p' && token_c[1] == 'r' && token_c[2] == 'i' && token_c[3] == 'n' && token_c[4] == 't' && token_c[5] == '\0') {
            i = token_end_idx;
            while (py[i] != '(' && py[i] != '\0') i++; // Ищем открывающую скобку
            
            if (py[i] == '(') {
                i++; // Пропускаем '('
                put_str("    printf(");
                
                if (py[i] == '"') {
                    // Обработка строки: print("hello") -> printf("%s", "hello")
                    put_str("\"%s\", ");
                    
                    putchar(py[i]); i++; // Вывод первой кавычки
                    while (py[i] != '"' && py[i] != '\0') { putchar(py[i]); i++; } // Вывод содержимого
                    if (py[i] == '"') { putchar(py[i]); i++; } // Вывод закрывающей кавычки
                } else {
                    // Обработка переменной: print(x) -> printf("%d", x)
                    put_str("\"%d\", ");
                    // Копируем имя переменной
                    while (py[i] != ')' && py[i] != '\n' && py[i] != '\r' && py[i] != '\0') { putchar(py[i]); i++; }
                }
                put_str(");\n");
            }
            
            // Пропускаем остаток строки до следующей команды
            while (py[i] != '\n' && py[i] != '\r' && py[i] != '\0') {
                i++;
            }
            continue;
        }

        //Обработка присваивания и ввода
        
        i = token_end_idx;
        while (py[i] == ' ' || py[i] == '\t') i++; // Пропускаем пробелы до '='

        if (py[i] == '=') {
            i++; // Пропускаем '='
            while (py[i] == ' ' || py[i] == '\t') i++; // Пропускаем пробелы после '='
            
            // Проверяем ввод: x = int(input()) -> scanf("%d", &x);
            if (py[i] == 'i' && py[i+1] == 'n' && py[i+2] == 't' && py[i+3] == '(' &&
                py[i+4] == 'i' && py[i+5] == 'n' && py[i+6] == 'p' && py[i+7] == 'u' && 
                py[i+8] == 't' && py[i+9] == '(' && py[i+10] == ')') 
            {
                // Генерируем C-код для ввода
                put_str("    // int "); put_str(token_c); put_str(";\n"); // Напоминание об объявлении
                put_str("    scanf(\"%d\", &"); put_str(token_c); put_str(");\n");
                i += 11; // Сдвигаем индекс после int(input())
            } else {
                 // Обычное присваивание: x = 10 -> x = 10;
                put_str("    "); 
                put_str(token_c);
                put_str(" = ");

                // Копируем выражение (правую часть) до конца строки
                while (py[i] != '\n' && py[i] != '\r' && py[i] != '\0') {
                    putchar(py[i]);
                    i++;
                }
                put_str(";\n");
            }
            continue;
        }
        
        // Если команда не распознана, сдвигаем индекс, чтобы избежать зацикливания
        i = line_start + 1; 
    }

    put_str("    return 0;\n");
    put_str("}\n");

    return 0;
}



// Выводит строку посимвольно с помощью putchar
void put_str(const char *s) {
    while (*s != '\0') {
        putchar(*s);
        s++;
    }
}

// Пропускает пробелы, табы, переводы строк и возвращает индекс первого значащего символа
int skip_sp(const char *buf, int i) {
    while (buf[i] == ' ' || buf[i] == '\t' || buf[i] == '\n' || buf[i] == '\r') {
        i++;
    }
    return i;
}

// Определяет текущий отступ (количество пробелов/табов)
int get_ind(const char *buf, int i, int *ind_chars) {
    *ind_chars = 0; // Сбрасываем счетчик отступа
    while (buf[i] == ' ' || buf[i] == '\t') {
        (*ind_chars)++; // Считаем символы отступа
        i++;
    }
    return i; // Возвращаем индекс первого не-отступа (начала команды)
}

// Читает токен (ключевое слово или имя переменной) до первого разделителя
int token(const char *buf, int i, char *token, int max_len) {
    int j = 0;
    while (buf[i] != ' ' && buf[i] != '\t' && 
           buf[i] != '=' && buf[i] != '>' && 
           buf[i] != '<' && buf[i] != ':' && 
           buf[i] != '(' && buf[i] != '\n' && buf[i] != '\r' && buf[i] != '\0' && j < max_len - 1) 
    {
        token[j++] = buf[i++];
    }
    token[j] = '\0'; // Завершаем токен
    return i;
}
