#include <stdio.h> 
#include <stdbool.h> 
#define MAX_INPUT 1000
#define MAX_BODY 500

char body_buf[MAX_BODY];
int body_len;

void put_str(const char *s); 
int skip_sp(const char *buf, int i);
int get_ind(const char *buf, int i, int *ind_chars);
int token(const char *buf, int i, char *token, int max_len);

int main() {
    char py[MAX_INPUT];
    int char_count = 0;
    int c;

    while ((c = getchar()) != EOF && char_count < MAX_INPUT - 1) {
        py[char_count++] = (char)c;
    }
    py[char_count] = '\0';
    put_str("#include <stdio.h>\n");
    put_str("\n");
    put_str("int main() {\n");
    int i = 0;
    char token_c[64]; 
    
    while (i < char_count) {
        i = skip_sp(py, i);
        if (py[i] == '\0') {
            break;
        }
        int line_start = i;
        i = token(py, i, token_c, 64);
        int token_end_idx = i; 

        while (py[i] == ' ' || py[i] == '\t') {
            i++;
        }
        
        if (token_c[0] == 'i' && token_c[1] == 'f' && token_c[2] == '\0') {
            put_str("    if (");
            i = token_end_idx; 
            while (py[i] != ':' && py[i] != '\0') {
                putchar(py[i]);
                i++;
            }
            put_str(") {\n");
            while (py[i] == ':' || py[i] == ' ' || py[i] == '\t' || py[i] == '\n' || py[i] == '\r') {
                i++;
            }
            int block_start_index = i;
            int init_ind;
            int body_start_index = get_ind(py, block_start_index, &init_ind);
            body_len = 0;
            int current_indentation = init_ind;
            int current_line_start = body_start_index;

            while (i < char_count) {
                while (py[i] != '\n' && py[i] != '\r' && py[i] != '\0') {
                    i++;
                }
                body_buf[body_len++] = ' ';
                body_buf[body_len++] = ' '; 
                body_buf[body_len++] = ' '; 
                body_buf[body_len++] = ' '; 
                
                int j;
                for (j = current_line_start; j < i; j++) {
                    if (body_len < MAX_BODY - 2) {
                        body_buf[body_len++] = py[j];
                    }
                }
                body_buf[body_len++] = ';'; 
                body_buf[body_len++] = '\n';
                while (py[i] == '\n' || py[i] == '\r') {
                    i++;
                }
                int next_line_indent_start = i;
                int next_ind;
                i = get_ind(py, i, &next_ind);
                if (next_ind < init_ind && i < char_count) {
                    break; 
                }
                current_line_start = i; 
            }
            body_buf[body_len] = '\0';
            put_str(body_buf);
            put_str("    }\n");
            continue;
        }


        if (token_c[0] == 'p' && token_c[1] == 'r' && token_c[2] == 'i' && token_c[3] == 'n' && token_c[4] == 't' && token_c[5] == '\0') {
            i = token_end_idx;
            i = token_end_idx;
            while (py[i] != '(' && py[i] != '\0') i++;
            if (py[i] == '(') {
                i++;
                put_str("    printf(");
                
                if (py[i] == '"') {
                    put_str("\"%s\", ");
                    
                    putchar(py[i]); i++;
                    while (py[i] != '"' && py[i] != '\0') { putchar(py[i]); i++; }
                    if (py[i] == '"') { putchar(py[i]); i++; }
                } else {
                    put_str("\"%d\", ");
                    while (py[i] != ')' && py[i] != '\n' && py[i] != '\r' && py[i] != '\0') { putchar(py[i]); i++; }
                }
                put_str(");\n");
            }
            
            while (py[i] != '\n' && py[i] != '\r' && py[i] != '\0') {
                i++;
            }
            continue;
        }
        i = token_end_idx;
        while (py[i] == ' ' || py[i] == '\t') i++;

        if (py[i] == '=') {
            i++; 
            while (py[i] == ' ' || py[i] == '\t') i++;
            
            if (py[i] == 'i' && py[i+1] == 'n' && py[i+2] == 't' && py[i+3] == '(' &&
                py[i+4] == 'i' && py[i+5] == 'n' && py[i+6] == 'p' && py[i+7] == 'u' && 
                py[i+8] == 't' && py[i+9] == '(' && py[i+10] == ')') 
            {
                put_str("    // int "); put_str(token_c); put_str(";\n"); 
                put_str("    scanf(\"%d\", &"); put_str(token_c); put_str(");\n");
                i += 11; 
            } else {
                put_str("    "); 
                put_str(token_c);
                put_str(" = ");

                while (py[i] != '\n' && py[i] != '\r' && py[i] != '\0') {
                    putchar(py[i]);
                    i++;
                }
                put_str(";\n");
            }
            continue;
        }
        i = line_start + 1; 
    }

    put_str("    return 0;\n");
    put_str("}\n");

    return 0;
}

void put_str(const char *s) {
    while (*s != '\0') {
        putchar(*s);
        s++;
    }
}
int skip_sp(const char *buf, int i) {
    while (buf[i] == ' ' || buf[i] == '\t' || buf[i] == '\n' || buf[i] == '\r') {
        i++;
    }
    return i;
}
int get_ind(const char *buf, int i, int *ind_chars) {
    *ind_chars = 0;
    int start = i;
    while (buf[i] == ' ' || buf[i] == '\t') {
        (*ind_chars)++;
        i++;
    }
    return i;
}

int token(const char *buf, int i, char *token, int max_len) {
    int j = 0;
    while (buf[i] != ' ' && buf[i] != '\t' && 
           buf[i] != '=' && buf[i] != '>' && 
           buf[i] != '<' && buf[i] != ':' && 
           buf[i] != '(' && buf[i] != '\n' && buf[i] != '\r' && buf[i] != '\0' && j < max_len - 1) 
    {
        token[j++] = buf[i++];
    }
    token[j] = '\0';
    return i;
}
