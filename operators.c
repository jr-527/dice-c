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

Furthermore, any time we perform an operation that can shrink an array, we need to check if
that can cause the array to have length 1. In that case, free the array and change type to
'1'.

I think we only need
___D1
___DD
___1D
*/

double T_at(Token d, int64_t n) {
    if (n < d.left || n >= d.left + d.len) {
        return 0.0;
    }
    return d.arr[n-d.left];
}

Token addD1(Token d, Token i) {
    // Memory: stack
    // Cannot shrink
    d.left += i.left;
    return d;
}

Token add1D(Token i, Token d) {
    // Memory: stack
    // Cannot shrink
    d.left += i.left;
    return d;
}

Token addDD(Token d1, Token d2) {
    // Memory: convolve reallocates + returns d1.arr, reallocates + frees d2.arr
    // Cannot shrink
    // size_t new_len;
    d1.arr = convolve(d1.arr, d1.len, d2.arr, d2.len, &(d1.len));
    //d1.len = new_len;
    d1.left += d2.left;
    //printf("addDD new len: %ld\n", new_len);
    return d1;
}

Token subD1(Token d, Token i) {
    // Memory: stack, safe
    // Cannot shrink
    d.left -= i.left;
    return d;
}

Token sub1D(Token i, Token d) {
    // Memory: in-place
    // Cannot shrink
    flip(d.arr, d.len);
    d.left = i.left-(d.left+d.len-1);
    return d;
}

Token mul1D(Token i, Token d) {
    // Memory: dynamic, either freed here or reallocated + returned by grow_by_int
    // Cannot shrink
    int64_t new_size;
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
    // Cannot shrink
    int64_t lower, upper;
    d1.arr = multiply_pmfs(d1.arr, d1.len, d2.arr, d2.len,
                           d1.left, d2.left, &lower, &upper);
    d1.left = lower;
    d1.len = upper-lower+1;
    return d1;
}

Token divD1(Token d1, Token i) {
    // Memory: divide_indices frees d1.arr, returns a new array
    // Shrinking is handled
    d1.arr = divide_indices(d1.arr, &(d1.len), &(d1.left), i.left);
    if (d1.len == 1) {
        free(d1.arr);
        d1.type = '1';
    }
    return d1;
}

Token divDD(Token d1, Token d2) {
    // Memory: divide_pmfs frees d1.arr, d2.arr, returns a new array
    // Shrinking is handled
    d1.arr = divide_pmfs(d1.arr, d1.len, d2.arr, d2.len, d1.left, d2.left,
                         &d1.left, &d1.len);
    if (d1.len == 1) {
        free(d1.arr);
        d1.type = '1';
    }
    return d1;
}

Token div1D(Token i, Token d) {
    // Memory: handled by divDD
    // Shrinking is handled by divDD
    i.type = 'D';
    i.arr = malloc(sizeof(double));
    i.len = 1;
    i.arr[0] = 1.0;
    return divDD(i,d);
}

Token equ11(Token i1, Token i2) {
    // Memory: stack, safe
    // Cannot shrink
    i1.left = (i1.left == i2.left);
    return i1;
}
Token neq11(Token i1, Token i2) {
    // Memory: stack, safe
    // Cannot shrink
    i1.left = (i1.left != i2.left);
    return i1;
}

Token equD1(Token d, Token i) {
    // Memory: reallocated + returned
    // Shrinking is handled
    //double* new_arr = malloc(2*sizeof(double));
    double p = 0.0;
    if (d.left <= i.left && i.left < d.left+d.len) {
        if (d.len == 1) {
            free(d.arr); // certain to be true, demote to integer
            i.left = 1;
            return i;
        }
        p = d.arr[i.left-d.left];
    } else {
        free(d.arr); // certain to be false, demote to integer
        i.left = 0;
        return i;
    }
    //free(d.arr);
    //d.arr = new_arr;
    d.arr = realloc(d.arr, 2*sizeof(double));
    d.arr[1] = p;
    d.arr[0] = 1.0-p;
    d.len = 2;
    d.left = 0;
    return d;
}
#define equ1D(i,d) equD1(d,i)
Token neqD1(Token d, Token i) {
    // Memory: managed by equD1
    // Shrinking: handled by equD1
    d = equD1(d, i);
    if (d.type == '1') {
        d.left = 1-d.left;
        return d;
    }
    double temp = d.arr[0];
    d.arr[0] = d.arr[1];
    d.arr[1] = temp;
    return d;
}
#define neq1D(x,y) neqD1(y,x)

Token equDD(Token d1, Token d2) {
    // Memory: d2 is freed, d1 is reallocated + returned
    // Shrinking is handled (input can't be singular -> only certain on disjoint)
    // WLOG d1.left <= d2.left
    if (d1.left > d2.left) {
        Token temp = d1;
        d1 = d2;
        d2 = temp;
    }
    if (d2.left >= d1.left+d1.len) {
        // distributions are disjoint
        free(d1.arr);
        free(d2.arr);
        d1.left = 0;
        d1.type = '1';
        return d1;
    } else if (d1.len == 1 && d2.len == 1 && d1.left == d2.left) {
        // two degenerate distributions at same point -> certainly equal
        free(d1.arr);
        free(d2.arr);
        d1.left = 1;
        d1.type = '1';
        return d1;
    }
    double sum = 0.0;
    double c = 0.0;
    for (int64_t i = 0; i < d2.len; i++) {
        // i is position in d2.arr
        // j is position in d1.arr
        int64_t j = i + (d2.left - d1.left);
        if (j >= d1.len) {
            break;
        }
        double y = d1.arr[j] * d2.arr[i] - c;
        double t = sum + y;
        c = (t-sum)-y;
        sum = t;
        // sum += d1.arr[j] * d2.arr[i];
    }
    free(d2.arr);
    d1.arr = realloc(d1.arr, 2*sizeof(double));
    d1.len = 2;
    d1.left = 0;
    d1.arr[1] = sum;
    d1.arr[0] = 1.0-sum;
    return d1;
}
Token neqDD(Token d1, Token d2) {
    // Memory: handled by neqDD
    // Shrinking: handled by equDD
    Token d = equDD(d1, d2);
    if (d.type == '1') {
        d.left = 1-d.left;
        return d;
    }
    double temp = d1.arr[0];
    d1.arr[0] = d1.arr[1];
    d1.arr[1] = temp;
    return d;
}

Token gre11(Token x, Token y) {
    // Memory: all stack memory
    // Cannot shrink
    x.left = (x.left > y.left);
    return x;
}
#define les11(x,y) gre11(y,x)
Token geq11(Token x, Token y) {
    // Memory: stack
    // Cannot shrink
    x.left = (x.left >= y.left);
    return x;
} //     x <= y   <->   y >= x
#define leq11(x, y) geq11(y, x)

Token greD1(Token d, Token i) {
    // Memory: reallocated and returned
    // Shrinking: handled
    // d > i
    double p;
    int64_t d_index = i.left - d.left;
    if (d_index >= d.len - 1) {
        // i is >= max possible value of d, so always false
        free(d.arr);
        i.left = 0;
        return i;
        //p = 0.0;
    } else if (d_index < 0) {
        // i is < min possible value of d, so always true
        free(d.arr);
        i.left = 1;
        return i;
        //p = 1.0;
    } else {
        // Could be smarter about this. If d.len = 1024*1024, d_index = 1024*1024-3,
        // then we would be better off just going backwards.
        ip_cumsum(d.arr, d.len);
        p = 1.0-d.arr[d_index];
    }
    d.arr = realloc(d.arr, 2*sizeof(double));
    d.arr[0] = 1.0-p;
    d.arr[1] = p;
    d.left = 0;
    d.len = 2;
    return d;
} //     i < d   <->   d > i
#define les1D(i,d) greD1(d,i)
Token geqD1(Token d, Token i) {
    // Memory: handled here/by greD1
    // shrinking is handled
    // d >= i
    if (d.left + d.len <= i.left) {
        // all d < i, so always false
        free(d.arr);
        i.left = 0;
        return i;
    } else if (d.left >= i.left) {
        // all d >= i, always true
        free(d.arr);
        i.left = 1;
        return i;
    }
    // Memory: handled by greD1
    double eq = T_at(d, i.left);
    d = greD1(d, i);
    if (d.type == '1') {
        d.arr = malloc(2*sizeof(double));
        d.type = 'D';
        d.arr[1] = d.left;
        d.arr[0] = 1-d.left;
        d.left = 0;
        d.len = 2;
    }
    d.arr[0] -= eq;
    d.arr[1] += eq;
    return d;
} //    i <= d  <->   d >= i
#define leq1D(i,d) geqD1(d,i)

Token gre1D(Token i, Token d) {
    // Memory: Reallocated and returned
    // Shrinking is handled
    // i > d
    double p;
    int64_t d_index = i.left - d.left;
    if (d_index >= d.len) {
        free(d.arr);
        i.left = 1;
        return i;
    } else if (d_index <= 0) {
        free(d.arr);
        i.left = 0;
        return i;
    } else {
        p = d.arr[d_index];
        ip_cumsum(d.arr, d.len);
        p = d.arr[d_index] - p;
    }
    d.arr = realloc(d.arr, 2*sizeof(double));
    d.arr[0] = 1.0-p;
    d.arr[1] = p;
    d.left = 0;
    d.len = 2;
    return d;
} //     i < d   <->  d > i
#define lesD1(i,d) gre1D(d,i)
Token geq1D(Token i, Token d) {
    // Memory: handled by gre1D
    // Shrinking is handled
    // i >= d
    if (i.left < d.left) {
        // i < all d, so always false
        free(d.arr);
        i.left = 0;
        return i;
    } else if (i.left >= d.left+d.len-1) {
        // i >= all d, so always true
        free(d.arr);
        i.left = 1;
        return i;
    }
    double eq = T_at(d, i.left);
    d = gre1D(i, d);
    if (d.type == '1') {
        d.arr = malloc(2*sizeof(double));
        d.type = 'D';
        d.arr[1] = d.left;
        d.arr[0] = 1-d.left;
        d.left = 0;
        d.len = 2;
    }
    d.arr[0] -= eq;
    d.arr[1] += eq;
    return d;
} //     d <= i  <->  i >= d
#define leqD1(d,i) geq1D(i,d)

Token greDD(Token d1, Token d2) {
    // Memory: d2 is freed, d1 is freed or reallocated + returned
    // Shrinking is handled
    // d1 > d2
    if (d1.left >= d2.left+d2.len) {
        // all d1 > all d2, certainly true
        free(d1.arr);
        free(d2.arr);
        d1.type = '1';
        d1.left = 1;
        return d1;
    } else if (d1.left+d1.len-1 <= d2.left) {
        // all d1 <= all d2, certainly false
        free(d1.arr);
        free(d2.arr);
        d1.type = '1';
        d1.left = 0;
        return d1;
    }
    double sum = 0.0;
    double c = 0.0;
    ip_cumsum(d1.arr, d1.len);
    for (int64_t n = d2.left; n < d2.left+d2.len; n++) {
        if (n >= d1.left+d1.len) {
            break;
        }
        // P(d1 > d2) = Sum_{n in d2} P(d1>i | d2=n) * P(d2=n) by law of total probability
        // and we do Kahan summation of that
        //double y = (1.0-T_at(d1, n))*T_at(d2, n) - c;
        double y = (1.0-T_at(d1, n)) * d2.arr[n-d2.left] - c;
        //double y = (1.0-d1.arr[i1])*d2.arr[i2] - c;
        double t = sum+y;
        c = (t-sum)-y;
        sum = t;
        // sum += (1.0-d1.arr[i1])*d2.arr[i2];
    }
    free(d2.arr);
    d1.arr = realloc(d1.arr, 2*sizeof(double));
    d1.len = 2;
    d1.left = 0;
    d1.arr[1] = sum;
    d1.arr[0] = 1.0-sum;
    return d1;
} //     x < y   <->   y > x
#define lesDD(x,y) greDD(y,x)

Token leqDD(Token d1, Token d2) {
    // Memory: d2 is freed, d1 is freed or reallocated + returned
    // Shrinking is handled
    // d1 <= d2
    if (d1.left >= d2.left+d2.len) {
        // all d1 > all d2, certainly false
        free(d1.arr);
        free(d2.arr);
        d1.type = '1';
        d1.left = 0;
        return d1;
    } else if (d1.left+d1.len-1 <= d2.left) {
        // all d1 <= all d2, certainly true
        free(d1.arr);
        free(d2.arr);
        d1.type = '1';
        d1.left = 1;
        return d1;
    }
    double sum = 0.0;
    double c = 0.0;
    ip_cumsum(d1.arr, d1.len);
    for (int64_t n = d2.left; n < d2.left+d2.len; n++) {
        if (n >= d1.left+d1.len) {
            break;
        }
        // P(d1 <= d2) = 1-P(d1 > d2) = 1-Sum_{n in d2} P(d1>i | d2=n) * P(d2=n) by law of total probability
        // and we do Kahan summation of that
        //double y = (1.0-T_at(d1, n))*T_at(d2, n) - c;
        double y = (1.0-T_at(d1, n)) * d2.arr[n-d2.left] - c;
        //double y = (1.0-d1.arr[i1])*d2.arr[i2] - c;
        double t = sum+y;
        c = (t-sum)-y;
        sum = t;
        // sum += (1.0-d1.arr[i1])*d2.arr[i2];
    }
    sum = 1.0-sum;
    free(d2.arr);
    d1.arr = realloc(d1.arr, 2*sizeof(double));
    d1.len = 2;
    d1.left = 0;
    d1.arr[1] = sum;
    d1.arr[0] = 1.0-sum;
    return d1;
}
#define geqDD(x,y) leqDD(y,x)

// @ operator - the (mostly) noncommutative multiplication operator
// It represents evaluating the LHS, converting the result to an integer,
// then adding together that many copies of the RHS.
// This is different from the * operator, which represents evaluating the LHS,
// evaluating the RHS, then multiplying.
Token of_1D(Token i, Token d) {
    // autoconvolve frees, and we free if we don't use it.
    // shrinking is handled
    if (i.left == 0) {
        free(d.arr);
        return i;
    }
    if (i.left < 0) {
        d.left = i.left*(d.left + d.len - 1);
    } else {
        d.left = i.left*d.left;
    }
    d.arr = autoconvolve(d.arr, d.len, i.left, &(d.len));
    return d;
}

// In this case it's the same as regular multiplication.
#define of_D1(x,y) mulD1(x,y)

Token of_DD(Token d1, Token d2) {
    // at_multiply_pmfs manages memory
    // cannot shrink (won't receive distribution that's always 0)
    int64_t upper;
    d1.arr = at_multiply_pmfs(d1.arr, d1.len, d2.arr, d2.len,
                              d1.left, d2.left, &(d1.left), &upper);
    d1.len = upper - d1.left + 1;
    return d1;
}

Token mod11(Token x, Token y) {
    // stack memory
    // cannot shrink
    x.left = x.left % y.left;
    return x;
}

Token modD1(Token d, Token x) {
    if (x.left == 1 || x.left == -1) {
        free(d.arr);
        x.left = 0;
        return x;
    }
    int64_t min = d.left % x.left;
    int64_t max = d.left % x.left;
    if (d.len < (x.left > 0 ? x.left : -x.left)) {
        for (int i = d.left; i < d.len; i++) {
            min = (d.left % i < min) ? d.left % i : min;
            max = (d.left % i > max) ? d.left % i : max;
        }
    } else if (x.left < 0) { // really <= -2
        min = x.left+1;
        max = 0;
    } else { // we exit earlier if x.left == 0, so >= 2
        min = 0;
        max = x.left-1;
    }
    int64_t new_len = max - min + 1;
    double* arr = calloc(new_len, sizeof(double));
    for (int i = 0; i < d.len; i++) {
        int64_t j = (d.left + i) % x.left;
        arr[j-min] += d.arr[i];
    }
    free(d.arr);
    d.arr = arr;
    d.len = new_len;
    d.left = min;
    return d;
}
