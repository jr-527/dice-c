#ifndef DEFS_C
#define DEFS_C

#include <stddef.h>
#include <stdint.h>

// This file contains common definitions and basic helpers used across various files.

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
    int64_t len;
    int64_t left;
    int64_t right;
    char type;
} Token;

#define OP_ADD '+'
#define OP_SUB '-'
#define OP_MUL '*'
#define OP_DIV '/'
#define OP_AT '@'
#define OP_MOD '%'
#define OP_POW '^'
#define OP_CON '|'
#define OP_GRE '>'
#define OP_LES '<'
#define OP_GEQ '~'
#define OP_LEQ '`'
#define OP_EQU '='
#define OP_NEQ ':'

// had to change some of these to avoid collisions
#define LPAREN '('
#define RPAREN ')'
#define COMMA ','
#define DICE_EXPRESSION '['
#define PMF ']'
#define CONSTANT '\\'
#define FUNCTION '{'

int input_is_operator(char x) {
    switch (x) {
    case '+': case '-':
    case '*': case '/':
    case '@': case '%':
    case '^': case '|':
    case '>': case '<':
    //case 'g': case 'l':
    case '=':// case 'n':
    case '!': // needed for some logic
        return 1;
    default:
        return 0;
    }
}

int byte_is_operator(char x) {
    switch (x) {
    case OP_ADD: case OP_SUB: case OP_DIV: case OP_MUL:
    case OP_MOD: case OP_POW: case OP_CON: case OP_AT:
    case OP_GRE: case OP_LES: case OP_GEQ: case OP_LEQ:
    case OP_EQU: case OP_NEQ: 
    case '!': // needed to make logic work
        return 1;
    default:
        return 0;
    }
}

int is_operator(Token t) {
    return byte_is_operator(t.type);
}

void print_token(Token t) {
    if (is_operator(t) || t.type == LPAREN || t.type == RPAREN) {
        printf(" %c ", t.type);
    } else if (t.type == DICE_EXPRESSION) {
        printf(" %ldd%ld ", t.left, t.right);
    } else if (t.type == PMF) {
        printf(" <D start:%ld,len:%ld> ", t.left, t.len);
    } else if (t.type == CONSTANT) {
        printf(" %ld ", t.left);
    } else if (t.type == FUNCTION) {
        printf(" func%ld ", t.left);
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
