// This code (heavily) modified from
// https://github.com/HighDiceRoller/icepool/blob/main/papers/icepool_preprint.pdf
// 
// The following license applies to this file only, I think:
// MIT License
// 
// Copyright © 2021-2024, Albert Julius Liu. All rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <tuple>
#include <map>
#include <stdint.h>
#include "drop.h"

#define min(x,y) (((x) > (y)) ? (y) : (x))

const size_t memory_limit = 4LL*1024*1024*1024/sizeof(double);

using std::tuple;
using std::map;

typedef struct Arr {
    int len;
    double* array;
} Arr;

typedef tuple<int,int,int> Triplet;
map<Triplet,Arr> cache_map;
size_t total_memory = 0;
size_t wasted_memory = 0;

Arr solve(const int faces, const int n, const int keep) {
    //printf("read  %d %d %d\n", faces, n, keep);
    // TODO: Try using a hash table instead of array.
    // Memory usage might be rough.
    map<Triplet,Arr>::iterator i = cache_map.find(Triplet(faces,n,keep));
    if (i != cache_map.end()) {
        return i->second;
    }
    int state;
    const int outlen = n*faces+1;
    Arr out;
    out.len = outlen;
    total_memory += outlen;
    if (total_memory >= memory_limit) {
        fprintf(stderr, "Current memory usage: %zd MB. Exiting to not use all your memory.\n",
                total_memory*sizeof(double)/(1024*1024)
        );
        exit(1);
    }
    out.array = (double*)calloc(outlen, sizeof(double));
    if (faces == 1) { // do this before the function call
        // From tests on reasonably large inputs, this contributes a tiny
        // fraction (0.7% or less) of our memory usage, so it isn't worth
        // trying to optimize this.
        // wasted_memory += outlen;;
        state = faces * min(n, keep);
        out.array[state] = 1.0;
        return out;
    }
    double binom_exp = 1.0;
    for (int k = 0; k < n+1; k++) {
        int mkk = min(keep, k);
        Arr tail = solve(faces-1, n-k, keep-mkk);
        for (int i = 0; i < tail.len; i++) {
            state = i + faces * mkk;
            double weight = tail.array[i];
            weight *= binom_exp;
            out.array[state] += weight;
        }
        binom_exp *= n-k;
        binom_exp /= k+1;
    }
    cache_map[Triplet(faces,n,keep)] = out;
    return out;
}

// I guess we want to copy the output, even though that wastes memory,
// because it's easier than 
// How do we handle free? It would be a pain if we freed the output,
// and copying it seems wasteful.
double* drop(const int faces, const int n, int keep, int64_t* leftptr, int64_t* lenptr) {
    int backwards = 0;
    if (keep < 0) {
        backwards = 1;
        keep = -keep;
    }
    Arr solution = solve(faces, n, keep);
    double* arr;
    int64_t left = 0;
    int64_t len;
    int64_t last = solution.len-1;
    while (solution.array[left] == 0.0) {
        left++;
    }
    while (solution.array[last] == 0.0) {
        last--;
    }
    len = last - left + 1;
    arr = (double*)malloc(len*sizeof(double));
    // My other code will eventually free arr, so it needs to be a copy.
    memcpy(arr, solution.array+left, len*sizeof(double));
    if (backwards) {
        double temp;
        for (int i = 0; i < len/2; i++) {
            temp = arr[i];
            arr[i] = arr[len-1-i];
            arr[len-1-i] = temp;
        }
    }
    double sum = 0.0;
    for (int i = 0; i < len; i++) {
        sum += arr[i];
    }
    for (int i = 0; i < len; i++) {
        arr[i] /= sum;
    }
    *leftptr = left;
    *lenptr = len;
    return arr;
}
