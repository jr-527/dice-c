#include "defs.c"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/*
#define MAX_TOKENS 128
#define TOKEN_LEN 128

char PARSE_BUF[MAX_TOKENS][TOKEN_LEN];
char INPUT_BUF[4096];

typedef struct Token {
    // if type is an operator, then this token is an operator (left/right are garbage)
    // if type is '1', then this token is the integer token.left (right is garbage)
    // if type is 'd', then this token is a die expression, ie (token.left)d(token.right)
    int left;
    int right;
    char type; // an operator, '1' for int
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
        if (char_is_operator(x) || x == '(' || x == ')') {
            if (token_i == 0) {
                if (token >= MAX_TOKENS) {
                    fprintf(stderr, "Input too big (More than %d tokens).\n", MAX_TOKENS);
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
                PARSE_BUF[token][0] = x;
                PARSE_BUF[token][1] = '\0';
                token += 1;
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
    char x = input[0];
    if (char_is_operator(x) || x == '(' || x == ')') {
        out.type = x;
        //printf("symbol: %c\n", x);
        return out;
    }
    out.type = '1'; // number
    int offset;
    if (!isdigit(input[0])) {
        fprintf(stderr, "Invalid input (type 1).\n");
        Exit(1);
    }
    sscanf(input, "%d%n", &(out.left), &offset);
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
        sscanf(input+offset+1, "%d%n", &(out.right), &count);
        if (count > 10) {
            fprintf(stderr, "Number too big.\n");
            Exit(1);
        }
        if (out.left == 0 || out.right == 0) {
            out.type = '1';
            out.left = 0;
        } else {
            out.type = 'd';
        }
    } else {
        if (strlen(input+offset) > 0) {
            fprintf(stderr, "Invalid input (type 3).\n");
            Exit(1);
        }
        //printf("integer: %d\n", out.left);
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
        if (j > 1 && TOKEN_BUF[j-1].type == '=') {
            // Things like ">=" are parsed as ">" "=", so if we encounter "=" right after
            // ">", we want to replace the ">" with "g" and decrement j.
            if (TOKEN_BUF[j-2].type == '>') {
                TOKEN_BUF[j-2].type = 'g';
                j -= 1;
                num_tokens_parsed -= 1;
                continue;
            } else if (TOKEN_BUF[j-2].type == '<') {
                TOKEN_BUF[j-2].type = 'l';
                j -= 1;
                num_tokens_parsed -= 1;
                continue;
            } else if (TOKEN_BUF[j-2].type == '=') {
                j -= 1;
                num_tokens_parsed -= 1;
                continue;
            } else if (TOKEN_BUF[j-2].type == '!') {
                TOKEN_BUF[j-2].type = 'n';
                j -= 1;
                num_tokens_parsed -= 1;
                continue;
            }
        }
        if (possible_unary && TOKEN_BUF[j-1].type == '-') {
            // unary negative
            TOKEN_BUF[j-1].type = '1';
            TOKEN_BUF[j-1].left = -1;
            TOKEN_BUF[j++].type = '*';
            num_tokens_parsed += 1;
        } else if (i > 0
                   && (TOKEN_BUF[j-1].type == '(' || TOKEN_BUF[j-1].type == 'd')
                   && TOKEN_BUF[j-2].type == ')') {
            // implict multiplication
            TOKEN_BUF[j] = TOKEN_BUF[j-1];
            TOKEN_BUF[j-1].type = '*';
            j += 1;
            num_tokens_parsed += 1;
        }
        possible_unary = is_operator(TOKEN_BUF[j-1]) || TOKEN_BUF[j-1].type == '(';
    }
    //for (int i = 0; i < num_tokens_parsed; i++) {
    //    print_token(TOKEN_BUF[i]);
    //}
    //printf("\n");
    return num_tokens_parsed;
}
