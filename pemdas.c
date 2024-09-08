#include "defs.c"
#include "operators.c"
#include "functions.c"
#include <stdio.h>

// This file handles order of operations and makes sure that the correct
// functions are applied in the correct order.

Token stack[128]; // RPN stack
Token queue[128]; // Queue used to convert to RPN

/**
 * Returns operator precedence, such that if the precedence of x is greater
 * than that of y, then precedence(x) > precedence(y)
 * 
 * \param t Token whose type is an OP_XXX constant
 * \return Returns an int representing precedence
 */
int precedence(const Token t) {
    switch (t.type) {
    case OP_POW: // exponentiation
        return 40;
    case OP_MUL: case OP_DIV:
    case OP_MOD: case OP_AT:
        return 30;
    case OP_ADD: case OP_SUB:
        return 20;
    case OP_GRE: case OP_LES:
    case OP_GEQ: case OP_LEQ:
    case OP_EQU: case OP_NEQ:
        return 10;
    }
    return 0;
}

Token addT(Token x, Token y) {
    if (x.type == PMF && y.type == CONSTANT) {
        return addD1(x, y);
    } else if (x.type == CONSTANT && y.type == PMF) {
        return add1D(x, y);
    } else if (x.type == CONSTANT && y.type == CONSTANT) {
        x.left += y.left;
        return x;
    } else if (x.type == PMF && y.type == PMF) {
        return addDD(x, y);
    }
    fprintf(stderr, "addT not implemented for %c and %c\n", x.type, y.type);
    Exit(1);
    return x;
}

Token subT(Token x, Token y) {
    if (x.type == CONSTANT && y.type == CONSTANT) {
        x.left -= y.left;
        return x;
    } else if (x.type == PMF && y.type == CONSTANT) {
        return subD1(x, y);
    } else if (x.type == CONSTANT && y.type == PMF) {
        return sub1D(x, y);
    }
    fprintf(stderr, "subT not implemented for %c and %c\n", x.type, y.type);
    Exit(1);
    return x;
}

Token mulT(Token x, Token y) {
    if (x.type == CONSTANT && y.type == CONSTANT) {
        x.left *= y.left;
        return x;
    } else if (x.type == PMF && y.type == CONSTANT) {
        return mulD1(x, y);
    } else if (x.type == CONSTANT && y.type == PMF) {
        return mul1D(x, y);
    } else if (x.type == PMF && y.type == PMF) {
        return mulDD(x, y);
    }
    fprintf(stderr, "mulT not implemented for %c and %c\n", x.type, y.type);
    Exit(1);
    return x;
}

Token divT(Token x, Token y) {
    if (x.type == CONSTANT && y.type == CONSTANT) {
        if (y.left == 0) {
            fprintf(stderr, "Cannot divide by zero\n");
            Exit(1);
        }
        x.left /= y.left;
        return x;
    } else if (x.type == PMF && y.type == CONSTANT) {
        if (y.left == 0) {
            fprintf(stderr, "Cannot divide by zero\n");
            Exit(1);
        }
        return divD1(x, y);
    } else if (x.type == CONSTANT && y.type == PMF) {
        return div1D(x, y);
    } else if (x.type == PMF && y.type == PMF) {
        return divDD(x, y);
    }
    fprintf(stderr, "divT not implemented for %c and %c\n", x.type, y.type);
    Exit(1);
    return x;
}

Token equT(Token x, Token y) {
    if (x.type == CONSTANT && y.type == CONSTANT) {
        return equ11(x, y);
    } else if (x.type == CONSTANT && y.type == PMF) {
        return equ1D(x, y);
    } else if (x.type == PMF && y.type == CONSTANT) {
        return equD1(x, y);
    } else if (x.type == PMF && y.type == PMF) {
        return equDD(x, y);
    }
    fprintf(stderr, "equT not implemented for %c and %c\n", x.type, y.type);
    Exit(1);
    return x;
}

Token neqT(Token x, Token y) {
   if (x.type == CONSTANT && y.type == CONSTANT) {
        return neq11(x, y);
    } else if (x.type == CONSTANT && y.type == PMF) {
        return neq1D(x, y);
    } else if (x.type == PMF && y.type == CONSTANT) {
        return neqD1(x, y);
    } else if (x.type == PMF && y.type == PMF) {
        return neqDD(x, y);
    }
    fprintf(stderr, "neqT not implemented for %c and %c\n", x.type, y.type);
    Exit(1);
    return x; 
}

Token greT(Token x, Token y) {
    if (x.type == CONSTANT && y.type == CONSTANT) {
        return gre11(x, y);
    } else if (x.type == PMF && y.type == CONSTANT) {
        return greD1(x, y);
    } else if (x.type == CONSTANT && y.type == PMF) {
        return gre1D(x, y);
    } else if (x.type == PMF && y.type == PMF) {
        return greDD(x, y);
    }
    fprintf(stderr, "greT not implemented for %c and %c\n", x.type, y.type);
    Exit(1);
    return x;
}

Token lesT(Token x, Token y) {
    if (x.type == CONSTANT && y.type == CONSTANT) {
        return les11(x, y);
    } else if (x.type == PMF && y.type == CONSTANT) {
        return lesD1(x, y);
    } else if (x.type == CONSTANT && y.type == PMF) {
        return les1D(x, y);
    } else if (x.type == PMF && y.type == PMF) {
        return lesDD(x, y);
    }
    fprintf(stderr, "lesT not implemented for %c and %c\n", x.type, y.type);
    Exit(1);
    return x;
}

Token geqT(Token x, Token y) {
    if (x.type == CONSTANT && y.type == CONSTANT) {
        return geq11(x,y);
    } else if (x.type == PMF && y.type == CONSTANT) {
        return geqD1(x,y);
    } else if (x.type == CONSTANT && y.type == PMF) {
        return geq1D(x,y);
    } else if (x.type == PMF && y.type == PMF) {
        return geqDD(x, y);
    }
    fprintf(stderr, "geqT not implemented for %c and %c\n", x.type, y.type);
    Exit(1);
    return x;
}

Token leqT(Token x, Token y) {
    if (x.type == CONSTANT && y.type == CONSTANT) {
        return leq11(x,y);
    } else if (x.type == PMF && y.type == CONSTANT) {
        return leqD1(x,y);
    } else if (x.type == CONSTANT && y.type == PMF) {
        return leq1D(x,y);
    } else if (x.type == PMF && y.type == PMF) {
        return leqDD(x, y);
    }
    fprintf(stderr, "leqT not implemented for %c and %c\n", x.type, y.type);
    Exit(1);
    return x;
}

Token of_T(Token x, Token y) {
    if (x.type == CONSTANT && y.type == CONSTANT) {
        x.left *= y.left;
        return x;
    } else if (x.type == PMF && y.type == CONSTANT) {
        return of_D1(x,y);
    } else if (x.type == CONSTANT && y.type == PMF) {
        return of_1D(x,y);
    } else if (x.type == PMF && y.type == PMF) {
        return of_DD(x,y);
    }
    fprintf(stderr, "of_T not implemented for %c and %c\n", x.type, y.type);
    Exit(1);
    return x;
}

Token modT(Token x, Token y) {
    if (x.type == CONSTANT && y.type == CONSTANT) {
        if (y.left == 0) {
            goto modby0;
        }
        return mod11(x,y);
    } else if (x.type == PMF && y.type == CONSTANT) {
        if (y.left == 0) {
            goto modby0;
        }
        return modD1(x,y);
    }
    fprintf(stderr, "modT not implemented for %c and %c\n", x.type, y.type);
    Exit(1);
    modby0:
    fprintf(stderr, "Cannot do modulo 0\n");
    Exit(1);
    return x;
}

/**
 * This implements the shunting yard algorithm. It takes valid expressions in
 * "standard" (infix) notation, made out of functions, operators, integer
 * constants, and parentheses, and reorders them into RPN (postfix) notation.
 * Note that this algorithm isn't sophisticated enough to work with a 4-level
 * stack, so we'll need a larger stack. We also have functions which can have
 * many arguments, so 4 wouldn't be enough anyways.
 * 
 * \param[in] tokens Array of Token structs, in "standard" (infix) notation
 * \param num_tokens Length of tokens
 * \return Returns the number of tokens placed in the RPN queue.
 */
int shunting_yard(const Token tokens[], const int num_tokens) {
    int q = 0;
    int s = 0;
    Token t;
    for (int i = 0; i < num_tokens; i++) {
        t = tokens[i];
        if (t.type == CONSTANT || t.type == DICE_EXPRESSION || t.type == DROPPER) { // number
            queue[q++] = t;
        } else if (t.type == FUNCTION) { // function
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
                    fprintf(stderr, "Mismatched parentheses!\n");
                    goto error;
                }
                queue[q++] = stack[--s];
            }
            s--;
        } else { // error
            fprintf(stderr, "Invalid type '%c'\n", t.type);
            goto error;
        }
    }
    while (s > 0) {
        queue[q++] = stack[--s];
    }
    return q;
    error:
    Exit(1);
    return q;
}

/**
 * Reads in tokens from the RPN queue and acts as an RPN calculator on them.
 * For example, if the queue is
 * [<5>, <2>, <+>, <2>, <*>],
 * then this returns <14>
 * 
 * \param q Length of the RPN queue
 * \return Returns the last output of the RPN calculations.
 */
Token reverse_polish(const int q) {
    debug("reverse_polish stack:");
    for (int i = 0; i < q; i++) {
        debug_print_token(queue[i]);
    }
    debug("\n");
    int s = 0;
    for (int i = 0; i < q; i++) {
        prepare_token(queue+i);
        Token next = queue[i];
        if (is_operator(next)) {
            switch(next.type) {
            case OP_ADD: next = addT(stack[s-2], stack[s-1]); break;
            case OP_MUL: next = mulT(stack[s-2], stack[s-1]); break;
            case OP_DIV: next = divT(stack[s-2], stack[s-1]); break;
            case OP_EQU: next = equT(stack[s-2], stack[s-1]); break;
            case OP_NEQ: next = neqT(stack[s-2], stack[s-1]); break;
            case OP_GRE: next = greT(stack[s-2], stack[s-1]); break;
            case OP_GEQ: next = geqT(stack[s-2], stack[s-1]); break;
            case OP_LES: next = lesT(stack[s-2], stack[s-1]); break;
            case OP_LEQ: next = leqT(stack[s-2], stack[s-1]); break;
            case OP_SUB: next = subT(stack[s-2], stack[s-1]); break;
            case OP_AT:  next = of_T(stack[s-2], stack[s-1]); break;
            case OP_MOD: next = modT(stack[s-2], stack[s-1]); break;
            default:
                fprintf(stderr, "type '%c' not implemented in reverse_polish\n", next.type);
                Exit(1);
                return next;
            }
            stack[s-2] = next;
            s -= 1;
        } else if (next.type == FUNCTION) {
            int64_t num_args = 0;
            Token return_value = apply_func(next, &stack[s-1], &num_args);
            stack[s-num_args] = return_value;
            s = s - num_args + 1;
            //fprintf(stderr, "functions are not implemented in reverse_polish\n");
            //Exit(1);
            //return next;
        } else {
            stack[s++] = next;
        }
    }
    return stack[0];
}

/**
 * Given an array of tokens representing an expression in "standard" (infix)
 * order, this evaluates the expression.
 * 
 * \param tokens Array of tokens in infix order representing an expression
 * \param len Length of tokens
 * \return Returns the value of the expression
 */
Token pemdas(Token tokens[], const int len) {
    return reverse_polish(shunting_yard(tokens, len));
}
