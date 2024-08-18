#include "defs.c"
#include "array_functions.c"
#define streq(x,y) !strcmp(x,y)

// This file implements and dispatches "functions" (expressions which use
// function syntax, like f(5,6) rather than 5*6 or 5d6)

Token zero(Token* stack_top, int64_t* num_args) { // 0
    *num_args = 0;
    (void)stack_top;
    Token out;
    out.left = 0;
    out.type = CONSTANT;
    return out;
}

Token identity(Token* stack_top, int64_t* num_args) { // 1
    *num_args = 1;
    return *stack_top;
}

Token twice(Token* stack_top, int64_t* num_args) { // 2
    *num_args = 1;
    Token out = *stack_top;
    out.left *= 2;
    return out;
}

Token divide(Token* stack_top, int64_t* num_args) { // 3
    *num_args = 2;
    Token out;
    int64_t left = stack_top[-1].left;
    int64_t right = stack_top[0].left;
    out.left = left/right;
    out.type = CONSTANT;
    return out;
}

Token (*func_ptrs[4]) (Token* st, int64_t* n) = {
    zero, identity, twice, divide
}; // array of function pointers :o

Token choose_func(char funcname[]) {
    Token out;
    out.type = FUNCTION;
    // TODO: Use binary search or something
    if (streq(funcname, "zero")) {
        out.left = 0;
    } else if (streq(funcname, "identity")) {
        out.left = 1;
    } else if (streq(funcname, "twice")) {
        out.left = 2;
    } else if (streq(funcname, "divide")) {
        out.left = 3;
    } else {
        fprintf(stderr, "No function named \"%s\"\n", funcname);
        Exit(1);
        return out;
    }
    return out;
}


Token apply_func(Token t, Token* stack_top, int64_t* num_args) {
    int64_t func_id = t.left;
    return func_ptrs[func_id](stack_top, num_args);
}
