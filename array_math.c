#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <string.h>
#include "pocketfft-master/pocketfft.h"

// This file contains various algorithms which are used on arrays.

#define complex128_t double complex
#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

void ip_unaligned_cpow(double* first, int n) {
    complex128_t x = first[0] + I*(first[1]);
    x = cpow(x, n);
    first[0] = creal(x);
    first[1] = cimag(x);
}

void accum_unaligned_cpow(double* first, double* out, int64_t n, double factor,
                          complex128_t rotate) {
    complex128_t x = first[0] + I*(first[1]);
    x = rotate * factor * cpow(x, n);
    out[0] += creal(x);
    out[1] += cimag(x);
}

void exponentiate_forward_rfft(double* arr, int64_t len, int n) {
    arr[0] = pow(arr[0], n);
    for (int64_t i = 1; i < len-1; i += 2) {
        ip_unaligned_cpow(arr+i, n);
    }
    if (len%2 == 0) {
        arr[len-1] = pow(arr[len-1], n);
    }
}

void accum_exp_forward_rfft(double* from, double* to, double factor, int64_t len,
                            int n, int64_t offset) {
    to[0] += factor * pow(from[0], n);
    int j = 0;
    for (int64_t i = 1; i < len-1; i+= 2) {
        j++;
        // multiply each element by by e^{-2pi I i/outlen} where I is sqrt(-1)
        complex128_t rotate = cos((-2*M_PI*j*offset)/len) + I*sin((-2*M_PI*j*offset)/len);
        accum_unaligned_cpow(from+i, to+i, n, factor, rotate);
    }
    if (len%2 == 0) {
        j++;
        complex128_t rotate = cos((-2*M_PI*j*offset)/len) + I*sin((-2*M_PI*j*offset)/len);
        to[len-1] += rotate*factor*pow(from[len-1], n);
    }

}

void print_rfft_forward(double* start, int64_t len) {
    printf("%f ", start[0]);
    for (int64_t i = 1; i < len-1; i += 2) {
        printf("%f%+fi ", start[i], start[i+1]);
    }
    if (len%2 == 0) {
        printf("(%f)", start[len-1]);
    }
    printf("\n");
}

void print_real_arr(double* start, int64_t len) {
    printf("[");
    for (int64_t i = 0; i < len; i++) {
        printf("%f ", start[i]);
    }
    printf("]\n");
}

double* ndm(int n, int m) {
    double* x = calloc(m*n, sizeof(double));
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
    //for (int i = m; i < n*m; i++) {
    //    x[i] = 0.0;
    //}
    rfft_plan plan = make_rfft_plan(n*m);
    rfft_forward(plan, x, 1.0);
    exponentiate_forward_rfft(x, n*m, n);
    rfft_backward(plan, x, 1.0/(n*m));
    if (!too_big) {
        val = pow(m,n);
        for (int64_t i = 0; i < n*m; i++) {
            x[i] = rint(x[i])/val;
        }
    }
    return x;
}

void flip(double* arr, int64_t len) {
    double temp;
    for (int64_t i = 0; i < len/2; i++) {
        temp = arr[i];
        arr[i] = arr[len-1-i];
        arr[len-1-i] = temp;
    }
}

double* autoconvolve(double* x, int64_t len, int64_t n, int64_t* outlenptr) {
    int negative = 0;
    if (n < 0) {
        negative = 1;
    }
    int64_t outlen = (len-1)*n + 1;
    *outlenptr = outlen;
    double* out = calloc(outlen, sizeof(double));
    memcpy(out, x, n*sizeof(double));
    free(x);
    rfft_plan plan = make_rfft_plan(outlen);
    rfft_forward(plan, out, 1.0);
    exponentiate_forward_rfft(out, outlen, n);
    rfft_backward(plan, out, 1.0/outlen);
    if (negative) {
        flip(out, outlen);
    }
    return out;
}

double* grow_by_int(double* arr, int64_t len, int n, int64_t* new_size) {
    if (n < 0) {
        flip(arr, len);
        n = -n;
    }
    *new_size = n*(len-1)+1;
    double* out = realloc(arr, (n*(len-1)+1)*sizeof(double));
    int64_t big_i = n*(len-1);
    int64_t small_i = len-1;
    while (big_i > 0) {
        out[big_i--] = out[small_i--];
        for (int j = 0; j < n-1; j++) {
            out[big_i--] = 0.0;
        }
    }
    return out;
}

double* convolve(double* x, int64_t xlen, double* y, int64_t ylen, int64_t* new_len) {
    // pad
    int64_t len = xlen + ylen;
    *new_len = len-1;
    x = realloc(x, sizeof(double)*len);
    y = realloc(y, sizeof(double)*len);
    for (int64_t i = xlen; i < len; i++) {
        x[i] = 0.0;
    }
    for (int64_t i = ylen; i < len; i++) {
        y[i] = 0.0;
    }
    // irfft(rfft(x) * rfft(y))
    rfft_plan plan = make_rfft_plan(len);
    rfft_forward(plan, x, 1.0);
    rfft_forward(plan, y, 1.0);
    x[0] *= y[0];
    for (int64_t i = 1; i < len-1; i += 2) {
        complex128_t temp = x[i] + I*x[i+1];
        temp *= y[i] + I*y[i+1];
        x[i] = creal(temp);
        x[i+1] = cimag(temp);
    }
    if (len % 2 == 0) {
        x[len-1] *= y[len-1];
    }
    free(y);
    rfft_backward(plan, x, 1.0/(len));
    return x;
}

void multiply_pmfs_bounds(int64_t xlen, int64_t ylen, int64_t xleft, int64_t yleft,
                          int64_t* lower, int64_t* upper) {
    int64_t xmax = xleft + xlen - 1;
    int64_t ymax = yleft + ylen - 1;
    int64_t v[4];
    int64_t min, max;
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

double* multiply_pmfs(double* x, int64_t xlen,
                     double* y, int64_t ylen,
                     int64_t xleft, int64_t yleft,
                     int64_t* lower, int64_t* upper) {
    // int64_t lower, upper;
    multiply_pmfs_bounds(xlen, ylen, xleft, yleft, lower, upper);
    double* out = calloc((*upper)-(*lower)+1, sizeof(double));
    // I know that *technically* out[0] need not be 0. If you've managed to
    // find some esoteric computer where this isn't an array of all 0.0,
    // then that's your problem.
    int64_t index;
    for (int64_t i = 0; i < xlen; i++) {
        for (int64_t j = 0; j < ylen; j++) {
            index = (i+xleft)*(j+yleft)-(*lower);
            out[index] += x[i]*y[j];
        }
    }
    free(x);
    free(y);
    return out;
}

double* at_multiply_pmfs(double* x, int64_t xlen, double* y, int64_t ylen,
                         int64_t xleft, int64_t yleft, int64_t* lower, int64_t* upper) {
    multiply_pmfs_bounds(xlen, ylen, xleft, yleft, lower, upper);
    //printf("at_multiply_pmfs lower:%ld, upper: %ld\n", *lower, *upper);
    int64_t outlen = (*upper)-(*lower)+1;
    double* out = calloc(outlen, sizeof(double));
    double* forward = calloc(outlen, sizeof(double));
    memcpy(forward, y, ylen*sizeof(double));
    rfft_plan plan = make_rfft_plan(outlen);
    rfft_forward(plan, forward, 1.0);
    //printf("forward\n");
    //print_rfft_forward(forward, outlen);
    // x[i]: P(x == n)
    // n > 0
    for (int64_t n = 1; n < xleft+xlen; n++) {
        int64_t i = n-xleft;
        // we need to offset this part in the "time" domain.
        // To avoid re-calculating FFTs, we rotate in the frequency domain
        //call signature: (from, to, factor, len, n, offset);
        //printf("n: %ld, offset: %ld\n", n, n*yleft-(*lower));
        accum_exp_forward_rfft(forward, out, x[i], outlen, n, n*yleft-(*lower));
        //printf("out after n=%ld, x[%ld]=%f:\n", n, i, x[i]);
        //print_rfft_forward(out, outlen);
    }
    if (xleft < 0) { // n < 0
        memset(forward, 0, outlen*sizeof(double));
        memcpy(forward, y, ylen*sizeof(double));
        flip(forward, outlen);
        rfft_forward(plan, forward, 1.0);
    }
    //int j = 0;
    for (int64_t n = xleft; n < 0 && n < xleft+xlen; n++) {
        //printf("n: %ld\n", n);
        int64_t i = n-xleft;
        int64_t right_bound;
        if (yleft == 0) {
            right_bound = 0;//n*(yleft+ylen-1);
            //printf("right_bound: %ld\n", right_bound);
        } else {
            // bound could be 0
            right_bound = n*yleft;
            //printf("right_bound: %ld\n", right_bound);
        }
        right_bound -= n+1; // Offsets the fact that convolutions shift.
        int64_t offset = right_bound - (*upper);
        //printf(" offset: %ld\n", offset);
        accum_exp_forward_rfft(forward, out, x[i], outlen, -n, offset);
    }
    rfft_backward(plan, out, 1.0/outlen);
    // n = 0
    if (xleft <= 0 && 0 < xleft+xlen) {
        int64_t i = -xleft;
        out[-(*lower)] += x[i];
        //for (int64_t j = 0; j < outlen; j++) {
        //    out[j] += x[i];
        //}
    }
    free(x);
    free(y);
    free(forward);
    //printf("out\n");
    //print_real_arr(out, outlen);
    return out;
}

double* divide_indices(double* x, int64_t* x_len, int64_t* x_left, int64_t n) {
    int swapped = 0;
    if (n < 0) {
        swapped = 1;
        n = -n;
    }
    int64_t xleft = *x_left;
    int64_t xlen = *x_len;
    int64_t end = xleft + xlen - 1;
    int64_t start = xleft/n;
    int64_t len = end/n-start+1;
    double* out = calloc(len, sizeof(double));
    for (int64_t i = 0; i < xlen; i++) {
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

double* divide_pmfs(double* x, int64_t xlen, double* y, int64_t ylen,
                    int64_t xleft, int64_t yleft, int64_t* outleftptr, int64_t* outlenptr) {
    // we calculate the minimum and maximum values of x/y
    // this code is nasty but O(1) so idc
    int64_t xmax = xleft + xlen - 1;
    int64_t ymax = yleft + ylen - 1;
    int64_t xbounds[2];
    int64_t ybounds[2];
    xbounds[0] = (xmax==0) ? -1 : xmax;
    xbounds[1] = (xleft==0) ? 1 : xleft;
    ybounds[0] = (ymax==0) ? -1 : ymax;
    ybounds[1] = (yleft==0) ? 1 : yleft;
    int64_t outleft = xbounds[0]/ybounds[0];
    int64_t outstop = xbounds[0]/ybounds[0];
    for (int i=0; i<2; i++) {
        for (int j=0; j<2; j++) {
            int64_t tmp = xbounds[i]/ybounds[j];
            outleft = (outleft < tmp) ? outleft : tmp;
            outstop = (outstop > tmp) ? outstop : tmp;
        }
    }
    // the loop. The idea is that we start with P(x/y==n) == 0 for all n,
    // then for all possible values i,j of x,y, we do
    // P(x/y == i/j) += P(x==i)*P(y==j)
    *outleftptr = outleft;
    int64_t outlen = outstop-outleft+1;
    *outlenptr = outlen;
    double* out = malloc(outlen*sizeof(double));
    for (int64_t i = 0; i < xlen; i++) {
        for (int64_t j = 0; j < ylen; j++) {
            if (j+yleft==0) {
                // division by zero
                if (y[j] == 0.0) {
                    // probability 0, we can skip this
                    continue;
                }
                // division by zero with nonzero probability
                fprintf(stderr, "Cannot divide by zero\n");
                Exit(1);
                return out;
            }
            out[(i+xleft)/(j+yleft)-outleft] += x[i]*y[j];
        }
    }
    free(x);
    free(y);
    return out;
}

void ip_cumsum(double* x, int64_t len) {
    if (len == 0) {
        return;
    }
    // Kahan summation
    double sum = x[0];
    double c = 0.0;
    for (int64_t i = 1; i < len; i++) {
        double y = x[i] - c;
        double t = sum + y;
        c = (t-sum)-y;
        sum = t;
        x[i] = sum;
    }
}

inline void ip_unaligned_cmul(double* x, double* y) {
    // x *= y
    complex128_t out = x[0] + I*(x[1]);
    out *= y[0] + I*(y[1]);
    x[0] = creal(out);
    x[1] = cimag(out);
}
