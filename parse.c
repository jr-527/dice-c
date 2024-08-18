#include "defs.c"
#include "functions.c"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// This handles string parsing.

/*
#define MAX_TOKENS 128
#define TOKEN_LEN 128

char PARSE_BUF[MAX_TOKENS][TOKEN_LEN];
char INPUT_BUF[4096];

typedef struct Token {
    // if type is an operator, then this token is an operator (left/right are garbage)
    // if type is CONSTANT, then this token is the integer token.left (right is garbage)
    // if type is DICE_EXPR, then this token is a die expression, ie (token.left)d(token.right)
    int left;
    int right;
    char type; // an operator, CONSTANT for int
} Token;

Token TOKEN_BUF[MAX_TOKENS];
*/
int split_tokens(char input[], int len) {
    // Splits the input into a bunch of tokens, ie
    // "1+(2+shoe)" ->
    // PARSE_BUF = ["1", "+", "(", "2", "+", "shoe", ")", (garbage)]
    // returns the number of strings returned.
    // returns -1 if the function "failed" due to a buffer overflow
    int token = 0; // Index of the token we're on
    int token_i = 0; // Index of next available spot in buffer
    for (int i = 0; i < len; i++) {
        char x = input[i];
        if (input_is_operator(x) || x == '(' || x == ')' || x == ',') {
            if (token_i == 0) {
                if (token >= MAX_TOKENS) {
                    fprintf(stderr, "Input too big (More than %d tokens).\n", MAX_TOKENS);
                    Exit(1);
                    return -1;
                }
                if (x == ',') {
                    fprintf(stderr, "Cannot start input with a comma\n");
                    Exit(1);
                    return -1;
                }
                PARSE_BUF[token][0] = x;
                PARSE_BUF[token][1] = '\0';
                token++;
            } else {
                if (token >= MAX_TOKENS-1) {
                    fprintf(stderr, "Input too big (More than %d tokens).\n", MAX_TOKENS);
                    Exit(1);
                    return -1;
                }
                //PARSE_BUF[token][token_i+1] = '\0'; // I think this line causes quiet failures.
                PARSE_BUF[token][token_i] = '\0';
                token += 1;
                if (x != ',') {
                    PARSE_BUF[token][0] = x;
                    PARSE_BUF[token][1] = '\0';
                    token += 1;
                }
                token_i = 0;
            }
            // add previous thing
            // add current thing
            // set prev=i+1;
        } else {
            if (token_i >= TOKEN_LEN-1) {
                return -1;
            }
            PARSE_BUF[token][token_i++] = x;
        }
    }
    PARSE_BUF[token][token_i] = '\0';
    if (PARSE_BUF[token][0] == '\0') {
        token -= 1;
    }
    return token+1;
}

Token str_to_token(char input[]) {
    // input: a null-terminated string that's either an operator,
    // an integer,
    // or something in the form "%dd%d"
    Token out;
    out.left = out.right = 0; // so GCC shuts up
    while (*input == ' ') {
        input++;
    }
    char x = input[0];
    if (input_is_operator(x) || x == '(' || x == ')') {
        out.type = x;
        return out;
    }
    out.type = CONSTANT; // number
    int offset;
    if (!isdigit(input[0])) {
        return choose_func(input);
        //fprintf(stderr, "Invalid input (type 1).\n");
        //Exit(1);
    }
    sscanf(input, "%ld%n", &(out.left), &offset);
    if (offset > 10) {
        fprintf(stderr, "Number too big.\n");
        Exit(1);
    }
    if (input[offset] == 'd') {
        int count;
        if (!isdigit(input[offset+1])) {
            fprintf(stderr, "Invalid input (type 2).\n");
            Exit(1);
        }
        sscanf(input+offset+1, "%ld%n", &(out.right), &count);
        if (count > 10) {
            fprintf(stderr, "Number too big.\n");
            Exit(1);
        }
        if (out.left == 0 || out.right == 0) {
            out.type = CONSTANT;
            out.left = 0;
        } else {
            out.type = DICE_EXPRESSION;
        }
    } else {
        if (input[offset] == ',') {
            offset++;
        }
        while (input[offset] == ' ') {
            offset++;
        }
        if (strlen(input+offset) > 0) {
            fprintf(stderr, "Invalid input (type 3).\n");
            Exit(1);
        }
    }
    return out;
    //scanf("%id%i", input, &(out.left), &(out.right));
}

int parse_token_main(int argc, char const* argv[]) {
    // split at arithmetic expressions
    int n = 0;
    for (int i = 1; i < argc; i++) {
        n += sprintf(INPUT_BUF + n, "%s", argv[i]);
    }
    int num_tokens = split_tokens(INPUT_BUF, n);
    int j = 0;
    int possible_unary = 1;
    int num_tokens_parsed = num_tokens;
    for (int i = 0; i < num_tokens; i++) {
        // printf("i: %d j: %d %s\n", i, j, PARSE_BUF[i]);
        TOKEN_BUF[j++] = str_to_token(PARSE_BUF[i]);
        if (j > 1 && TOKEN_BUF[j-1].type == OP_EQU) {
            // Things like ">=" are parsed as ">" "=", so if we encounter "=" right after
            // ">", we want to replace the ">" with "g" and decrement j.
            if (TOKEN_BUF[j-2].type == OP_GRE) {
                TOKEN_BUF[j-2].type = OP_GEQ;
                j -= 1;
                num_tokens_parsed -= 1;
                continue;
            } else if (TOKEN_BUF[j-2].type == OP_LES) {
                TOKEN_BUF[j-2].type = OP_LEQ;
                j -= 1;
                num_tokens_parsed -= 1;
                continue;
            } else if (TOKEN_BUF[j-2].type == OP_EQU) {
                j -= 1;
                num_tokens_parsed -= 1;
                continue;
            } else if (TOKEN_BUF[j-2].type == '!') {
                TOKEN_BUF[j-2].type = OP_NEQ;
                j -= 1;
                num_tokens_parsed -= 1;
                continue;
            }
        }
        if (possible_unary && TOKEN_BUF[j-1].type == OP_SUB) {
            // unary negative
            TOKEN_BUF[j-1].type = CONSTANT;
            TOKEN_BUF[j-1].left = -1;
            TOKEN_BUF[j++].type = OP_MUL;
            num_tokens_parsed += 1;
        } else if (i > 0
                   && (TOKEN_BUF[j-1].type==LPAREN || TOKEN_BUF[j-1].type==DICE_EXPRESSION)
                   && TOKEN_BUF[j-2].type == RPAREN) {
            // implict multiplication
            TOKEN_BUF[j] = TOKEN_BUF[j-1];
            TOKEN_BUF[j-1].type = OP_MUL;
            j += 1;
            num_tokens_parsed += 1;
        }
        possible_unary = is_operator(TOKEN_BUF[j-1]) || TOKEN_BUF[j-1].type == LPAREN;
    }
    // for (int i = 0; i < num_tokens_parsed; i++) {
        // print_token(TOKEN_BUF[i]);
    // }
    return num_tokens_parsed;
}
