#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "defs.c"
#include "term_size.c"
#include "plot.c"
#include "parse.c"
#include "pemdas.c"

Token main_parse(int argc, char const *argv[]) {
    int n = parse_token_main(argc, argv);
    Token t = pemdas(TOKEN_BUF, n);
    return t;
}

void main_plot(Token t) {
    int rows, cols;
    get_term_size(&rows, &cols);
    if (rows > 10000 || rows <= 0 || cols > 10000 || cols <= 0) {
        // we're probably printing to a file or something
        rows = 30;
        cols = 80;
    }
    //fprintf(stderr, "rows: %d, cols: %d\n", rows, cols);
    draw(rows, cols, t.arr, t.left, t.len);
    return;
}

void handle_main(int argc, char const *argv[]) {
    Token t = main_parse(argc, argv);
    if (t.type == '1' || (t.type == 'D' && t.len==1)) {
        printf("answer is always %ld\n", t.left);
    } else {
        main_plot(t);
        free(t.arr);
    }
}

void interactive_mode() {
    char interactive_buf[1024];
    char const *fake_argv[2] = {NULL, interactive_buf}; // keeps the warnings happy
    while (1) {
        fprintf(stderr, "Enter your input. Enter q to quit.\n");
        if (fgets(interactive_buf, 1024, stdin) == NULL) {
            fprintf(stderr, "Error reading input.\n");
            continue;
        }
        int len = strlen(interactive_buf);
        if (len == 1) {
            continue;
        }
        if (interactive_buf[len-1] != '\n') {
            fprintf(stderr, "Error reading input.\n");
            continue;
        }
        interactive_buf[len-1] = '\0';
        if (len == 2 && (interactive_buf[0]=='q' || interactive_buf[0]=='Q')) {
            return;
        }
        //printf("%s\n", fake_argv[0]);
        handle_main(2, fake_argv);
    }
}

int main(int argc, char const *argv[]) {
    if (argc < 2) {
        exit_flag = 1;
        interactive_mode();
        return 0;
    }
    handle_main(argc, argv); 
    return 0;
}
