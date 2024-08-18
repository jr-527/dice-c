#include <stdio.h>
#include <stdint.h>
#include <math.h>

// This file contains the math behind various functions that work on arrays.
// They should represent more advanced options than the stuff in array_math.c,
// which should represent operators and "literals". The distinction is mostly
// for organizational purposes.

void order_stat(double* arr, int64_t len, int64_t num, int64_t pos) {
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
    double cumulative = 0.0;
    double comp = 0.0;
    for (int64_t i = 0; i < len; i++) {
        // Kahan summation
        double y = arr[i] - comp;
        double t = cumulative + y;
        comp = (t - cumulative) - y;
        cumulative = t;

        double t1 = 1.0-cumulative;
        double t3 = 1.0-cumulative+arr[i];
        double t4 = cumulative-arr[i];
        double bin_coeff = 1.0;
        double out = 0.0;
        printf("cumulative: %f\n", cumulative);
        for (int64_t j = 0; j < num-pos+1; j++) {
            double temp = pow(t1, j) * pow(cumulative, num-j) - pow(t3, j) * pow(t4, num-j);
            out += temp * bin_coeff;
            bin_coeff *= (num-j)/(j+1);
        }
        arr[i] = out;
    }
}
/*
int64_t min(int64_t x, int64_t y) {
    return (x > y) ? y : x;
}

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

int main() {
    double in[] = {0.2, 0.2, 0.2, 0.2, 0.2};
    order_stat(in,5, 5, 4);
    printf("[%f, %f, %f, %f, %f]\n", in[0], in[1], in[2], in[3], in[4]);
    return 0;
}
*/
