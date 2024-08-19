#include <stdio.h>
#include <stdint.h>
#include <float.h>
#include <math.h>

// This file contains the math behind various functions that work on arrays.
// They should represent more advanced options than the stuff in array_math.c,
// which should represent operators and "literals". The distinction is mostly
// for organizational purposes.

/**
 * Computes order statistic in-place.
 * 
 * \param[in,out] arr: Input array, representing a PMF
 * \param len: length of arr
 * \param num: Number of samples
 * \param pos: Which order statistic, ie 1 for least, num for greatest.
 * Can do negative indexing, like in Python.
 */
void arr_order_stat(double* arr, const int64_t len, const int64_t num, int64_t pos) {
    // in place
    // arr, len: the array, its length
    // num: number of times we 'sample' arr
    // we keep the pos sample in the order. 1 -> min, num -> max
    if (pos < 0) {
        pos = num + pos + 1;
    }
    if (num == 1 && pos == 1) {
        return;
    }
    #if LDBL_MANT_DIG == 64
    long double cumulative = 0.0;
    #else
    double cumulative = 0.0;
    double comp = 0.0;
    #endif
    for (int64_t i = 0; i < len; i++) {
        #if LDBL_MANT_DIG == 64
        cumulative += arr[i];
        #else
        // Kahan summation
        double y = arr[i] - comp;
        double t = cumulative + y;
        comp = (t - cumulative) - y;
        cumulative = t;
        #endif
        double t1 = 1.0-cumulative;
        double t3 = 1.0-cumulative+arr[i];
        double t4 = cumulative-arr[i];
        double bin_coeff = 1.0; // exceeds 2**64; have to use double
        double out = 0.0;
        for (int64_t j = 0; j < num-pos+1; j++) {
            double temp = pow(t1, j) * pow(cumulative, num-j) - pow(t3, j) * pow(t4, num-j);
            out += temp * bin_coeff;
            bin_coeff *= (num-j)/((double)(j+1));
        }
        arr[i] = out;
    }
}
/*
WIP
double* drop(int64_t count, int64_t faces, int64_t n) {
    // n > 0 -> drop highest n
    // n < 0 -> drop lowest n
    double* out;
    int64_t size;
    if (n < 0) {
        // (count-n)*faces - (count-n) + 1 = (count-n)*(faces-1)+1
        size = (count+n)*(faces-1)+1
    } else {
        size = (count-n)*(faces-1)+1;
    }
}

double solve(int64_t faces, int64_t n, int64_t keep) {
    if (faces == 1) {
        int64_t state = faces * min(n, keep);
    }
    // use binary tree map for cache (lexicographical order)
    // use arrays of pair structs to store data? or just arrays with offset?
}
*/
