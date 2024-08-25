#ifndef FUNCTIONS_C
#define FUNCTIONS_C
#include "defs.c"
#include "array_functions.c"
#include "drop.h"

// This file implements and dispatches "functions" (expressions which use
// function syntax, like f(5,6) rather than 5*6 or 5d6)

// when the user types func(a,b,c), we have:
// stack_top[0] == c
// stack_top[-1] == b
// stack_top[-2] == a

/**
 * Represents "advantage", the distribution of the maximum of two samples
 * User usage: adv(x)
 * 
 * \param[in,out] stack_top The top of the RPN stack
 * \param[out] num_args Place to store the number of arguments popped off the stack
 */
Token adv(Token* stack_top, int64_t* num_args) {
    *num_args = 1;
    Token t = *stack_top;
    if (t.type == CONSTANT) {
        return t;
    }
    if (t.type != PMF) {
        fprintf(stderr, "Invalid argument for adv: '%c'\n", t.type);
        Exit(1);
        return t;
    }
    arr_order_stat(t.arr, t.len, 2, 2);
    return t;
}

/**
 * Represents "disadvantage", the distribution of the minimum of two samples
 * User usage: adv(x)
 *
 * \param[in,out] stack_top The top of the RPN stack
 * \param[out] num_args Place to store the number of arguments popped off the stack
 */
Token dis(Token* stack_top, int64_t* num_args) {
    *num_args = 1;
    Token t = *stack_top;
    if (t.type == CONSTANT) {
        return t;
    }
    if (t.type != PMF) {
        fprintf(stderr, "Invalid argument for dis: '%c'\n", t.type);
        Exit(1);
        return t;
    }
    arr_order_stat(t.arr, t.len, 2, 1);
    return t;
}

/**
 * Calculates order statistics (see array_order_stat in array_functions.c)
 * This function is clever and automatically swaps its second and third arguments
 * because I can never remember which is which, and it must hold that
 * trials >= position, so if trials < position we can just swap the two.
 * User usage: order_stat(distribution, n1, n2)
 *
 * \param[in,out] stack_top Pointer to the top of the RPN stack
 * \param[out] num_args Place to store the number of arguments popped off the stack
 */
Token order_stat(Token* stack_top, int64_t* num_args) {
    *num_args = 3;
    Token t = stack_top[-2];
    if (t.type == CONSTANT) {
        return t;
    }
    if ((t.type != PMF) || (stack_top[-1].type != CONSTANT)
                        || (stack_top[0].type != CONSTANT)) {
        fprintf(stderr, "Invalid types: order_stat('%c', '%c', '%c')\n",
                t.type, stack_top[-1].type, stack_top[0].type);
        Exit(1);
        return t;
    }
    int64_t trials = stack_top[0].left;
    int64_t position = stack_top[-1].left;
    // We want 1 <= position <= trials, also with negative indexing.
    // It's hard to remember which is which, so if the order is wrong we just
    // DWIM and swap them.
    if (trials < position) {
        int64_t temp = trials;
        trials = position;
        position = temp;
    }
    if (position < 0) {
        position = trials + position;
    }
    if ((1 > position) || (position > trials)) {
        fprintf(stderr, "Illegal values for order_stat(.., trials=%ld, position=%ld)\n",
                trials, position);
        Exit(1);
        return t;
    }
    //printf("order_stat(.., trials=%ld, position=%ld)\n", trials, position);
    arr_order_stat(t.arr, t.len, trials, position);
    return t;
}
/**
 * Calculates things along the line of 4d6dl.
 * User usage: drop(faces, total, keep)
 *
 * \param[in,out] stack_top Pointer to the top of the RPN stack
 * \param[out] num_args Place to store the number of arguments popped off the stack
 *
  /
Token drop_func(Token* stack_top, int64_t* num_args) {
    *num_args = 3;
    Token keep = stack_top[0];
    Token total = stack_top[-1];
    Token faces = stack_top[-2];
    if (keep.type != CONSTANT || total.type != CONSTANT || faces.type != CONSTANT) {
        fprintf(stderr, "drop can only take integer arguments\n");
        Exit(1);
        return faces;
    }
    faces.arr = drop(total.left, faces.left, keep.left, &faces.left, &faces.len);
    faces.type = PMF;
    return faces;
}
*/

/**
 * Struct used to associate a function (pointer) with a string representing
 * the function's name.
 */
typedef struct FuncTuple {
    char* name;
    Token (*func)(Token* st, int64_t* n);
} FuncTuple;

FuncTuple func_arr[] = { // keep this array sorted, for convenience
    {"adv", adv}, {"advantage", adv},
    {"dis", dis}, {"disadvantage", dis},
    //{"drop", drop_func},
    {"order", order_stat}, {"order_stat", order_stat},
};

/**
 * This looks up a function by name in our array of functions, so that we can
 * later call the correct function without needing a ton of else-ifs.
 * 
 * \param funcname The name of the function to look up.
 * \return A token that keeps track of the relevant function.
 */
Token choose_func(const char funcname[]) {
    Token out;
    out.left = -1;
    out.type = FUNCTION;
    // If this ever gets big I could use binary/interpolation search or
    // something but that doesn't matter for now.
    for (int i = 0; i < (int)(sizeof(func_arr)/sizeof(FuncTuple)); i++) {
        if (strcmp(func_arr[i].name, funcname) == 0) {
            out.left = i;
        }
    }
    if (out.left == -1) {
        fprintf(stderr, "No function named \"%s\"\n", funcname);
        Exit(1);
        return out;
    }
    return out;
}

/**
 * This applies the function that was identified by choose_func.
 * 
 * \param t A function token that was returned by choose_func
 * \param[in,out] stack_top The top of the RPN stack
 * \param[out] num_args Location to store the number of items this function
 * pops off the stack.
 */
Token apply_func(const Token t, Token* stack_top, int64_t* num_args) {
    int64_t func_id = t.left;
    return func_arr[func_id].func(stack_top, num_args);
}
#endif
