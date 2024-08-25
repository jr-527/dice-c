#ifndef DEFS_C
#define DEFS_C

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// This file contains common definitions and basic helpers used across various files.

/**
 * This struct represents one "token" while evaluating user input.
 * It works a lot like a class and has different "subclasses." Different
 * fields have different meanings depending on the "subclass" and may
 * be unused depending on the subclass. The type field should be one
 * of the macro constants defined in defs.c
 * 
 * The following list describes each "subclass" as well as all of the fields
 * which that "subclass" uses.
 * 
 * constants:
 *  - type CONSTANT
 *  - left: The value of this constant, ie 3 to represent the integer 3
 * 
 * operators: Use is_operator(token) or byte_is_operator(token.type) to check this.
 *  - type OP_XXX, ie OP_ADD, representing the "+" operator. 
 * 
 * parentheses:
 *  - type LPAREN or RPAREN, for "(", ")"
 * 
 * dice expressions: Here, 3d6 means to roll 3 six-sided dice and add up their results.
 *  - type DICE_EXPRESSION
 *  - left: The number of dice, ie 3 in 3d6
 *  - right: The number of faces on each die, ie 6 in 3d6.
 * 
 * distribution: A calculated probability mass function
 *  - type PMF
 *  - len: The length of arr
 *  - left: Value corresponding to arr[0], so if left is 5, then arr[0] is the
 *          probability that this distribution is equal to 5, arr[1] for 6, etc.
 *  - arr: Pointer to array storing the probability mass function.
 * 
 * function: A pre-defined function
 *  - type FUNCTION
 *  - left: Index of the function. Used by functions.c to dispatch the correct function.
 *
 * "drop" dice expression: Used to handle expressions like "4d6dl"
 *  - type DROPPER
 *  - left: same as regular dice expressions
 *  - right: same as regular dice expressions
 *  - len: extra value, number of things to drop (negative for dropping highest)
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
#define OP_AT  '@'
#define OP_MOD '%'
#define OP_POW '^'
#define OP_CON '|'
#define OP_GRE '>'
#define OP_LES '<'
#define OP_GEQ '~'
#define OP_LEQ '`'
#define OP_EQU '='
#define OP_NEQ ':'

// had to change some of these to avoid collisions with the names of functions.
// Originally, DICE_EXPRESSION was 'D', CONSTANT was '1', function was 'f',
// which is why the various add1D functions are named that way

#define LPAREN '('
#define RPAREN ')'
#define COMMA ','
// This was originally 'd'
#define DICE_EXPRESSION '['
// This was originally 'D', hence the names of functions like addDD
#define PMF ']'
// This was originally '1', hence the names of functions like mul1D
#define CONSTANT '\\'
#define FUNCTION '{'
// Needed to make some logic work
#define DROPPER ';'

/**
 * Returns true if the character, when input by a user, represents a binary operator,
 * false otherwise.
 */
int input_is_operator(const char x) {
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

/**
 * Returns true if the input represents a binary operator, ie
 * byte_is_operator(mytoken.type)
 */
int byte_is_operator(const char x) {
    switch (x) {
    case OP_ADD: case OP_SUB: case OP_DIV: case OP_MUL:
    case OP_MOD: case OP_POW: case OP_CON: case OP_AT:
    case OP_GRE: case OP_LES: case OP_GEQ: case OP_LEQ:
    case OP_EQU: case OP_NEQ: 
    case '!': // needed to make some logic work
        return 1;
    default:
        return 0;
    }
}

/**
 * Returns true if the provided token represents a binary operator
 */
int is_operator(const Token t) {
    return byte_is_operator(t.type);
}

/**
 * Debug helper function that prints a string representation of a token
 */
void print_token(const Token t) {
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

int exit_flag = 0; // If true, say something when exiting.
/**
 * Function to exit in case of error that could cause memory leak (most of them)
 */
void Exit(int n) {
    if (exit_flag) {
        fprintf(stderr, "Irrecoverable error. Exiting.\n");
        #ifdef _WIN32
        system("pause");
        #endif
    }
    exit(n);
}

// Arbitrary constant limiting how many tokens the user can input at a time.
// I figured that 128 is high enough that it won't be a problem.
// In the input "12 + someFunc(3d3/2)", the tokens are "12", "+", "someFunc",
// "(", "3d3",i "/", "2", ")"
#define MAX_TOKENS 128
// Arbitrary constant defining the maximum length of a token in characters.
// I figured that 128 is high enough that it won't be a problem.
#define TOKEN_LEN 128

// Buffer where tokens are placed during parsing.
Token TOKEN_BUF[MAX_TOKENS*2];
// Buffer where strings are placed during parsing.
char PARSE_BUF[MAX_TOKENS][TOKEN_LEN];
// Buffer where the user's input is placed.
char INPUT_BUF[4096];

// plotting
// Length of string buffer used in plotting. Limits the length of each row in
// the ASCII art plot.
#define PLOT_BUF_LEN 1024
// Buffer used in plotting that contains rows prior to being sent to printf
char PLOT_BUF[PLOT_BUF_LEN];
// Buffer used in plotting.
double DATA_BUF[PLOT_BUF_LEN];
// Number of columns to the left of the plot area
#define LEFT_OFFSET 1
// Number of columns to the right of the plot area
#define RIGHT_OFFSET 12

#endif
