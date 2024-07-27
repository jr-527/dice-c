#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "defs.c"
#include "term_size.c"
#include "plot.c"
#include "parse.c"
#include "pemdas.c"
#define N 62831

Token main_parse(int argc, char const *argv[]) {
    int n = parse_token_main(argc, argv);
    Token t = pemdas(TOKEN_BUF, n);
    return t;
}

void main_plot(Token t) {
    int rows, cols;
    get_term_size(&rows, &cols);
    //printf("rows: %d, cols: %d\n", rows, cols);
    //printf("(plot label TBD)\n");
    draw(rows, cols, t.arr, t.left, t.len);
    return;
    double data1[N];
    //double* data1 = malloc(N*sizeof(double));
    for (int i = 0; i < N; i++) {
        data1[i] = sin(((double)i)/10000)+1;
        //data1[i] = ((double)i)/N;
    }
    draw(rows, cols, data1, 0, N);
    //free(data1);
    //double data[] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    //draw(rows, cols, data, 0, 10);
}

int main(int argc, char const *argv[]) {
    Token t = main_parse(argc, argv);
    if (t.type == '1' || (t.type == 'D' && t.len==1)) {
        printf("answer is always %d\n", t.left);
    } else {
        main_plot(t);
        free(t.arr);
    }
    return 0;
}
