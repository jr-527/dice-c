#ifndef DEFS_C
#define DEFS_C

#include <stddef.h>
#include <stdint.h>

// token parsing
/*
type    Desc    Used attributes (not incl type)
'1'     int     left                  (left is value)
'd'     die     left, right
'D'     dist    left,        len, arr (left is start)
'f'     func    left
op.     op.

Operator        Description
+, *, -, /,     Same as dice.py
^, %, @,        Same
>, <, =         Same
g, l, n         >=, <=, != (\geq, \leq, \neq)
|               Hopefully conditioning, not sure how to implement.
*/
typedef struct Token {
    double* arr;
    size_t len;
    int left;
    int right;
    char type;
} Token;

int char_is_operator(char x) {
    switch (x) {
        case '+': case '-':
        case '*': case '/':
        case '@': case '%':
        case '^': case '|':
        case '>': case '<':
        case 'g': case 'l':
        case '=': case 'n':
        case '!': // needed to make logic work
            return 1;
        default:
            return 0;
    }
}

int is_operator(Token t) {
    return char_is_operator(t.type);
}

void print_token(Token t) {
    if (is_operator(t) || t.type == '(' || t.type == ')') {
        printf(" %c ", t.type);
    } else if (t.type == 'd') {
        printf(" %dd%d ", t.left, t.right);
    } else if (t.type == 'D') {
        printf(" <D start:%d,len:%ld> ", t.left, t.len);
    } else if (t.type == '1'){
        printf(" %d ", t.left);
    } else {
        printf(" <%c> ", t.type);
    }
}

int exit_flag = 0;
void Exit(int n) {
    if (exit_flag) {
        fprintf(stderr, "Irrecoverable error. Exiting.\n");
        #ifdef _WIN32
        system("pause");
        #endif
    }
    exit(n);
}

#define MAX_TOKENS 128
#define TOKEN_LEN 128

Token TOKEN_BUF[MAX_TOKENS*2];
char PARSE_BUF[MAX_TOKENS][TOKEN_LEN];
char INPUT_BUF[4096];

// memory management
#define WORKING_MEM_SIZE 2
uint8_t* WORKING_MEM[WORKING_MEM_SIZE];
size_t WORKING_SIZES[WORKING_MEM_SIZE];


// plotting
#define PLOT_BUF_LEN 1024
char PLOT_BUF[PLOT_BUF_LEN];
double DATA_BUF[PLOT_BUF_LEN];
#define LEFT_OFFSET 1
#define RIGHT_OFFSET 12

#endif