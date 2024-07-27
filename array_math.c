#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include "pocketfft-master/pocketfft.h"

#define complex128_t double complex

void ip_unaligned_cpow(double* first, int n) {
    complex128_t x = first[0] + I*(first[1]);
    x = cpow(x, n);
    first[0] = creal(x);
    first[1] = cimag(x);
}

void exponentiate_forward_rfft(double* arr, size_t len, int n) {
    arr[0] = pow(arr[0], n);
    for (size_t i = 1; i < len-1; i += 2) {
        ip_unaligned_cpow(arr+i, n);
    }
}

void print_rfft_forward(double* start, size_t len) {
    for (size_t i = 1; i < len-1; i += 2) {
        printf("%f%+fi ", start[i], start[i+1]);
    }
    printf("\n");
}

void print_real_arr(double* start, size_t len) {
    printf("arr:\n");
    for (size_t i = 0; i < len; i++) {
        printf("%f ", start[i]);
    }
    printf("\n");
}

double* ndm(int n, int m) {
    double* x = malloc(m*n*sizeof(double));
    int too_big = log2(n)*m > 52;
    // when n=m=150, n^m is too big to store as a double.
    double val;
    for (int i = 0; i < m; i++) {
        if (too_big) {
            x[i] = 1.0/m;
        } else {
            x[i] = 1.0;
        }
    }
    //if (!too_big) {
    //    val = pow(m,n);
    //}
    for (int i = m; i < n*m; i++) {
        x[i] = 0.0;
    }
    rfft_plan plan = make_rfft_plan(n*m);
    rfft_forward(plan, x, 1.0);
    exponentiate_forward_rfft(x, n*m, n);
    rfft_backward(plan, x, 1.0/(n*m));
    if (!too_big) {
        val = pow(m,n);
        for (int i = 0; i < n*m; i++) {
            x[i] = rint(x[i])/val;
        }
    }
    return x;
}

void flip(double* arr, size_t len) {
    double temp;
    for (size_t i = 0; i < len/2; i++) {
        temp = arr[i];
        arr[i] = arr[len-1-i];
        arr[len-1-i] = temp;
    }
}

double* grow_by_int(double* arr, size_t len, int n, long int* new_size) {
    if (n < 0) {
        flip(arr, len);
        n = -n;
    }
    *new_size = n*(len-1)+1;
    double* out = realloc(arr, (n*(len-1)+1)*sizeof(double));
    size_t big_i = n*(len-1);
    size_t small_i = len-1;
    while (big_i > 0) {
        out[big_i--] = out[small_i--];
        for (int j = 0; j < n-1; j++) {
            out[big_i--] = 0.0;
        }
    }
    return out;
}

double* convolve(double* x, size_t xlen, double* y, size_t ylen, size_t* new_len) {
    // pad
    size_t len = xlen + ylen;
    *new_len = len-1;
    x = realloc(x, sizeof(double)*len);
    y = realloc(y, sizeof(double)*len);
    for (size_t i = xlen; i < len; i++) {
        x[i] = 0.0;
    }
    for (size_t i = ylen; i < len; i++) {
        y[i] = 0.0;
    }
    // irfft(rfft(x) * rfft(y))
    rfft_plan plan = make_rfft_plan(len);
    rfft_forward(plan, x, 1.0);
    rfft_forward(plan, y, 1.0);
    x[0] *= y[0];
    for (size_t i = 1; i < len-1; i += 2) {
        complex128_t temp = x[i] + I*x[i+1];
        temp *= y[i] + I*x[i+1];
        x[i] = creal(temp);
        x[i+1] = cimag(temp);
    }
    free(y);
    rfft_backward(plan, x, 1.0/(len));
    //printf("convolve out\n");
    //print_real_arr(x, len);
    return x;
}

void multiply_pmfs_bounds(size_t xlen, size_t ylen, int xleft, int yleft,
                          int* lower, int* upper) {
    int xmax = xleft + xlen - 1;
    int ymax = yleft + ylen - 1;
    int v[4];
    int min, max;
    v[0] = xleft * yleft;
    v[1] = xleft * ymax;
    v[2] = xmax * yleft;
    v[3] = xmax * ymax;
    min = max = v[0];
    for (int i = 1; i < 4; i++) {
        min = (v[i] < min) ? v[i] : min;
        max = (v[i] > max) ? v[i] : max;
    }
    *lower = min;
    *upper = max;
}

double* multiply_pmfs(double* x, size_t xlen,
                     double* y, size_t ylen,
                     int xleft, int yleft,
                     int* lower, int* upper) {
    // long int lower, upper;
    multiply_pmfs_bounds(xlen, ylen, xleft, yleft, lower, upper);
    double* out = calloc((*upper)-(*lower)+1, sizeof(double));
    // I know that *technically* out[0] need not be 0. If you've managed to
    // find some esoteric computer where this isn't an array of all 0.0,
    // then that's your problem.
    int index;
    for (int i = 0; i < (int)xlen; i++) {
        for (int j = 0; j < (int)ylen; j++) {
            index = (i+xleft)*(j+yleft)-(*lower);
            out[index] += x[i]*y[j];
        }
    }
    free(x);
    free(y);
    return out;
}

double* divide_indices(double* x, size_t* x_len, int* x_left, int n) {
    int swapped = 0;
    if (n < 0) {
        swapped = 1;
        n = -n;
    }
    int xleft = *x_left;
    size_t xlen = *x_len;
    int end = xleft + xlen - 1;
    int start = xleft/n;
    int len = end/n-start+1;
    double* out = calloc(len, sizeof(double));
    for (int i = 0; i < (int)xlen; i++) {
        out[(xleft+i)/n - start] += x[i];
    }
    if (swapped) {
        start = -(start+len-1);
        flip(out, len);
    }
    *x_left = start;
    *x_len = len;
    free(x);
    return out;
}

inline void ip_unaligned_cmul(double* x, double* y) {
    // x *= y
    complex128_t out = x[0] + I*(x[1]);
    out *= y[0] + I*(y[1]);
    x[0] = creal(out);
    x[1] = cimag(out);
}
/*
int main() {
    double* x = ndm(3, 6);
    return 0;
}
*/
