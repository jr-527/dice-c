#include "defs.c"
#include "array_math.c"

void prepare_token(Token* t) {
    if (t->type != 'd') {
        return;
    }
    t->type = 'D';
    t->arr = ndm(t->left, t->right);
    t->len = (t->left)*(t->right-1)+1;
    // Converts dice expressions ('d') to distributions ('D') in-place.
    // Does nothing for all other token types.
}

/* The following functions should be in the form
Token ___TS(Token t1, Token t2)
where ___ is the operation, eg mul,
T is the type of t1,
S is the type of t2.
This is where all the memory management happens. Any time a type 'D' is input, either:
(1) free(d.arr) is called somewhere
(2) d.arr is set to realloc(d.arr, ...) somewhere
(3) d.arr is modified in-place

I think we only need
___D1
___DD
___1D
*/
Token addD1(Token d, Token i) {
    // Memory: stack, safe
    d.left += i.left;
    return d;
}

Token add1D(Token i, Token d) {
    // Memory: stack, safe
    d.left += i.left;
    return d;
}

Token addDD(Token d1, Token d2) {
    // Memory: convolve reallocates + returns d1.arr, reallocates + frees d2.arr
    // size_t new_len;
    d1.arr = convolve(d1.arr, d1.len, d2.arr, d2.len, &(d1.len));
    //d1.len = new_len;
    d1.left += d2.left;
    //printf("addDD new len: %ld\n", new_len);
    return d1;
}

Token subD1(Token d, Token i) {
    // Memory: stack, safe
    d.left -= i.left;
    return d;
}

Token sub1D(Token i, Token d) {
    // Memory: in-place, safe
    flip(d.arr, d.len);
    d.left = i.left-(d.left+d.len-1);
    return d;
}

Token mul1D(Token i, Token d) {
    // Memory: dynamic, either freed here or reallocated + returned by grow_by_int
    long int new_size;
    if (i.left == 0) {
        free(d.arr);
        i.left = 0;
        return i;
    }
    d.arr = grow_by_int(d.arr, d.len, i.left, &new_size);
    if (i.left > 0) {
        d.left *= i.left;
    } else {
        d.left = (d.left + d.len - 1)*i.left;
    }
    d.len = new_size;
    return d;
}
#define mulD1(d,i) mul1D(i,d)

Token mulDD(Token d1, Token d2) {
    // Memory: multiply_pmfs frees d1.arr, d2.arr, and returns a new array
    int lower, upper;
    d1.arr = multiply_pmfs(d1.arr, d1.len, d2.arr, d2.len,
                           d1.left, d2.left, &lower, &upper);
    d1.left = lower;
    d1.len = upper-lower+1;
    return d1;
}

Token divD1(Token d1, Token i) {
    // Memory: divide_indices frees d1.arr, returns a new array
    d1.arr = divide_indices(d1.arr, &(d1.len), &(d1.left), i.left);
    return d1;
}
