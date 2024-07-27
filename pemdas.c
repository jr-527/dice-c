#include "defs.c"
#include "operators.c"
#include <stdio.h>

Token stack[128];
Token queue[128];

int precedence(Token t) {
    switch (t.type) {
    case '^': // exponentiation
        return 40;
    case '*':
    case '/':
    case '@':
    case '%':
        return 30;
    case '+':
    case '-':
        return 20;
    case '<': // others TBD. Probably replace ">=" with '.' or something, etc
    case '>':
    case '=':
        return 10;
    }
    return 0;
}

Token addT(Token x, Token y) {
    if (x.type == 'D' && y.type == '1') {
        return addD1(x, y);
    } else if (x.type == '1' && y.type == 'D') {
        return add1D(x, y);
    } else if (x.type == '1' && y.type == '1') {
        x.left += y.left;
        return x;
    } else if (x.type == 'D' && y.type == 'D') {
        return addDD(x, y);
    }
    printf("addT not implemented\n");
    exit(1);
    return x;
}

Token subT(Token x, Token y) {
    if (x.type == '1' && y.type == '1') {
        x.left -= y.left;
        return x;
    } else if (x.type == 'D' && y.type == '1') {
        return subD1(x, y);
    } else if (x.type == '1' && y.type == 'D') {
        return sub1D(x, y);
    }
    printf("subT not implemented\n");
    exit(1);
    return x;
}

Token mulT(Token x, Token y) {
    if (x.type == '1' && y.type == '1') {
        x.left *= y.left;
        return x;
    } else if (x.type == 'D' && y.type == '1') {
        return mulD1(x, y);
    } else if (x.type == '1' && y.type == 'D') {
        return mul1D(x, y);
    } else if (x.type == 'D' && y.type == 'D') {
        return mulDD(x, y);
    }
    printf("mulT not implemented\n");
    exit(1);
    return x;
}

Token divT(Token x, Token y) {
    if (x.type == '1' && y.type == '1') {
        x.left /= y.left;
        return x;
    } else if (x.type == 'D' && y.type == '1') {
        if (y.left == 0) {
            printf("Cannot divide by zero");
            exit(1);
        }
        return divD1(x, y);
    }
    printf("divT not implemented");
    exit(1);
    Token out;
    out.left = x.left/y.left;
    return out;
}

/*
int is_operator(Token t) {
    switch (t.type) {
        case '+': case '-':
        case '*': case '/':
        case '@': case '%':
        case '^': case '|':
        case '>': case '<':
        case 'g': case 'l':
        case '=': case 'n':
             return 1;
        default:
            return 0;
    }
}
*/

int shunting_yard(Token tokens[], int num_tokens) {
    int q = 0;
    int s = 0;
    Token t;
    for (int i = 0; i < num_tokens; i++) {
        t = tokens[i];
        if (t.type == '1' || t.type == 'd') { // number
            queue[q++] = t;
        } else if (t.type == 'f') { // function
            stack[s++] = t;
        } else if (is_operator(t)) { // operator
            while (
                (s>0) && is_operator(stack[s-1]) && (stack[s-1].type != '(')
                && (precedence(stack[s-1]) >= precedence(t)) // all ops are left-assoc
            ) {
                queue[q++] = stack[--s];
            }
            stack[s++] = t;
        } else if (t.type == '(') {
            stack[s++] = t;
        } else if (t.type == ')') {
            while (stack[s-1].type != '(') {
                // assert stack is not empty (optional)
                if (s == 0) {
                    printf("Parenthese mismatch!\n");
                    goto error;
                }
                queue[q++] = stack[--s];
            }
            s--;
            if (stack[s-1].type == 'f') {
                printf("Functions are not implemented yet\n");
                goto error;
            }
        } else { // error
            printf("Invalid type %c\n", t.type);
            goto error;
        }
    }
    while (s > 0) {
        queue[q++] = stack[--s];
    }
    error:
        return q;
}

Token reverse_polish(int q) {
    int s = 0;
    for (int i = 0; i < q; i++) {
        prepare_token(queue+i);
        Token next = queue[i];
        if (is_operator(next)) {
            if (next.type == '+') {
                next = addT(stack[s-2], stack[s-1]);
            } else if (next.type == '*') {
                next = mulT(stack[s-2], stack[s-1]);
            } else if (next.type == '/') {
                next = divT(stack[s-2], stack[s-1]);
            } else {
                next = subT(stack[s-2], stack[s-1]);
            }
            stack[s-2] = next;
            s -= 1;
        } else if (next.type == 'f') {
            printf("not implemented!");
            return next;
        } else {
            stack[s++] = next;
        }
    }
    return stack[0];
}

Token pemdas(Token tokens[], int len) {
    /*
    int n = shunting_yard(tokens, len);
    for (int i = 0; i < n; i++) {
        print(queue[i]);
    }
    printf("\n");
    */
    return reverse_polish(shunting_yard(tokens, len));
}
