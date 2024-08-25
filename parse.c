#include "defs.c"
#include "functions.c"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// This handles string parsing.

/**
 * Splits the input into a bunch of tokens, ie
 * `"1+(2+shoe)"` -> `"1", "+", "(", "2", "+", "shoe", ")"`
 * The tokens are written into PARSE_BUF such that PARSE_BUF[i] is a
 * null-terminated string containing one token.
 * 
 * \param[in] input Input string
 * \param len Length of input
 * \return The number of tokens parsed
 */
int split_tokens(const char input[], const int len) {
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

/**
 * Converts a string into a token representing that string.
 * 
 * \param input String
 * \return Token representing that string
 */
Token str_to_token(const char input[]) {
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
        //printf("after \"end\" of token: \"%c\"\n", input[offset+1+count]);
        if (count > 10) {
            fprintf(stderr, "Number too big.\n");
            Exit(1);
        }
        if (out.left == 0 || out.right == 0) {
            out.type = CONSTANT;
            out.left = 0;
        } else {
            int64_t dropper_val = 0;
            const char first_char = input[offset+1+count];
            if (first_char == 'd' || first_char == 'k') {
                const char second_char = input[offset+2+count];
                if (second_char == 'h' || second_char == 'l') {
                    if (!isdigit(input[offset+3+count])) {
                        dropper_val = 1;
                    } else {
                        sscanf(input+offset+3+count, "%ld", &dropper_val);
                    }
                } else {
                    fprintf(stderr, "Invalid input (in dropping handler).\n");
                    Exit(1);
                }
                if (first_char == 'd') {
                    dropper_val = out.left - dropper_val;
                } else {
                    dropper_val *= -1;
                }
                if (second_char == 'h') {
                    dropper_val *= -1;
                }
                out.len = dropper_val;
                out.type = DROPPER;
            } else {
                out.type = DICE_EXPRESSION;
            }
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

/**
 * Parses input in the same format as main(int argc, char* argv[])
 * 
 * \param argc - argc as passed to main
 * \param argv - argv as passed to main
 * \return Returns the number of tokens parsed
 */
int parse_token_main(const int argc, char const* argv[]) {
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
