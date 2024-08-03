#include "defs.c"
#include "operators.c"
#include <stdio.h>

Token stack[128];
Token queue[128];

int precedence(Token t) {
    switch (t.type) {
    case '^': // exponentiation
        return 40;
    case '*': case '/':
    case '@': case '%':
        return 30;
    case '+': case '-':
        return 20;
    case '<': case 'l':
    case '>': case 'g':
    case '=': case 'n':
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
    fprintf(stderr, "addT not implemented for %c and %c\n", x.type, y.type);
    Exit(1);
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
    fprintf(stderr, "subT not implemented for %c and %c\n", x.type, y.type);
    Exit(1);
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
    fprintf(stderr, "mulT not implemented for %c and %c\n", x.type, y.type);
    Exit(1);
    return x;
}

Token divT(Token x, Token y) {
    if (x.type == '1' && y.type == '1') {
        if (y.left == 0) {
            fprintf(stderr, "Cannot divide by zero\n");
            Exit(1);
        }
        x.left /= y.left;
        return x;
    } else if (x.type == 'D' && y.type == '1') {
        if (y.left == 0) {
            fprintf(stderr, "Cannot divide by zero\n");
            Exit(1);
        }
        return divD1(x, y);
    } else if (x.type == '1' && y.type == 'D') {
        return div1D(x, y);
    } else if (x.type == 'D' && y.type == 'D') {
        return divDD(x, y);
    }
    fprintf(stderr, "divT not implemented for %c and %c\n", x.type, y.type);
    Exit(1);
    return x;
}

Token equT(Token x, Token y) {
    if (x.type == '1' && y.type == '1') {
        return equ11(x, y);
    } else if (x.type == '1' && y.type == 'D') {
        return equ1D(x, y);
    } else if (x.type == 'D' && y.type == '1') {
        return equD1(x, y);
    } else if (x.type == 'D' && y.type == 'D') {
        return equDD(x, y);
    }
    fprintf(stderr, "equT not implemented for %c and %c\n", x.type, y.type);
    Exit(1);
    return x;
}

Token neqT(Token x, Token y) {
   if (x.type == '1' && y.type == '1') {
        return neq11(x, y);
    } else if (x.type == '1' && y.type == 'D') {
        return neq1D(x, y);
    } else if (x.type == 'D' && y.type == '1') {
        return neqD1(x, y);
    } else if (x.type == 'D' && y.type == 'D') {
        return neqDD(x, y);
    }
    fprintf(stderr, "neqT not implemented for %c and %c\n", x.type, y.type);
    Exit(1);
    return x; 
}

Token greT(Token x, Token y) {
    if (x.type == '1' && y.type == '1') {
        return gre11(x, y);
    } else if (x.type == 'D' && y.type == '1') {
        return greD1(x, y);
    } else if (x.type == '1' && y.type == 'D') {
        return gre1D(x, y);
    } else if (x.type == 'D' && y.type == 'D') {
        return greDD(x, y);
    }
    fprintf(stderr, "greT not implemented for %c and %c\n", x.type, y.type);
    Exit(1);
    return x;
}

Token lesT(Token x, Token y) {
    if (x.type == '1' && y.type == '1') {
        return les11(x, y);
    } else if (x.type == 'D' && y.type == '1') {
        return lesD1(x, y);
    } else if (x.type == '1' && y.type == 'D') {
        return les1D(x, y);
    } else if (x.type == 'D' && y.type == 'D') {
        return lesDD(x, y);
    }
    fprintf(stderr, "lesT not implemented for %c and %c\n", x.type, y.type);
    Exit(1);
    return x;
}

Token geqT(Token x, Token y) {
    if (x.type == '1' && y.type == '1') {
        return geq11(x,y);
    } else if (x.type == 'D' && y.type == '1') {
        return geqD1(x,y);
    } else if (x.type == '1' && y.type == 'D') {
        return geq1D(x,y);
    } else if (x.type == 'D' && y.type == 'D') {
        return geqDD(x, y);
    }
    fprintf(stderr, "geqT not implemented for %c and %c\n", x.type, y.type);
    Exit(1);
    return x;
}

Token leqT(Token x, Token y) {
    if (x.type == '1' && y.type == '1') {
        return leq11(x,y);
    } else if (x.type == 'D' && y.type == '1') {
        return leqD1(x,y);
    } else if (x.type == '1' && y.type == 'D') {
        return leq1D(x,y);
    } else if (x.type == 'D' && y.type == 'D') {
        return leqDD(x, y);
    }
    fprintf(stderr, "leqT not implemented for %c and %c\n", x.type, y.type);
    Exit(1);
    return x;
}

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
            printf("Invalid type '%c'\n", t.type);
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
            switch(next.type) {
            case '+': next = addT(stack[s-2], stack[s-1]); break;
            case '*': next = mulT(stack[s-2], stack[s-1]); break;
            case '/': next = divT(stack[s-2], stack[s-1]); break;
            case '=': next = equT(stack[s-2], stack[s-1]); break;
            case 'n': next = neqT(stack[s-2], stack[s-1]); break;
            case '>': next = greT(stack[s-2], stack[s-1]); break;
            case 'g': next = geqT(stack[s-2], stack[s-1]); break;
            case '<': next = lesT(stack[s-2], stack[s-1]); break;
            case 'l': next = leqT(stack[s-2], stack[s-1]); break;
            case '-': next = subT(stack[s-2], stack[s-1]); break;
            default:
                fprintf(stderr, "type '%c' not implemented in reverse_polish\n", next.type);
                Exit(1);
                return next;
            }
            stack[s-2] = next;
            s -= 1;
        } else if (next.type == 'f') {
            fprintf(stderr, "functions are not implemented in reverse_polish\n");
            Exit(1);
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
