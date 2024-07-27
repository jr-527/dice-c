#include "defs.c"
#include <stdio.h>
//#include <string.h>
/*
#define PLOT_BUF_LEN 1024
char PLOT_BUF[PLOT_BUF_LEN];
double DATA_BUF[PLOT_BUF_LEN];
#define LEFT_OFFSET 1
#define RIGHT_OFFSET 12
*/
void draw_horiz(int main_cols) {
    for (int i = 0; i < LEFT_OFFSET-1; i++) {
        PLOT_BUF[i] = ' ';
    }
    PLOT_BUF[LEFT_OFFSET-1] = '+';
    for (int i = LEFT_OFFSET; i < LEFT_OFFSET+main_cols; i++) {
        PLOT_BUF[i] = '-';
    }
    PLOT_BUF[LEFT_OFFSET+main_cols] = '+';
    PLOT_BUF[LEFT_OFFSET+main_cols+1] = '\0';
    //PLOT_BUF[LEFT_OFFSET+main_cols+1] = '\0';
    printf("%s\n", PLOT_BUF);
    fflush(stdout);
}

// left: The label to put on the left (negative for no label)
// right: same but on the right
void draw_main(int main_cols, int r, double left, double right) {
    if (left < 0) {
        for (int i = 0; i < LEFT_OFFSET-1; i++) {
            PLOT_BUF[i] = ' ';
        }
        PLOT_BUF[LEFT_OFFSET-1] = '|';
    } else {
        sprintf(PLOT_BUF, "%1.6f|", left);
    }
    for (int i = LEFT_OFFSET; i < LEFT_OFFSET+main_cols; i++) {
        if (DATA_BUF[i-LEFT_OFFSET] >= r) {
            if (DATA_BUF[i-LEFT_OFFSET] >= r+.666666666667) {
                PLOT_BUF[i] = '@';
            } else if (DATA_BUF[i-LEFT_OFFSET] >= r+.333333333333) {
                PLOT_BUF[i] = 'x';
            } else if (DATA_BUF[i-LEFT_OFFSET] >= r+.1) {
                PLOT_BUF[i] = '_';
            } else {
                PLOT_BUF[i] = ' ';
            }
        } else {
            PLOT_BUF[i] = ' ';
        }
    }
    PLOT_BUF[LEFT_OFFSET+main_cols] = '|';
    if (right < 0) {
        PLOT_BUF[LEFT_OFFSET+main_cols+1] = '\0';
    } else {
        sprintf(PLOT_BUF+LEFT_OFFSET+main_cols+1, "%1.9f", right);
    }
    printf("%s\n", PLOT_BUF);
    fflush(stdout);
}

int fit_data(double* data, int len, int main_cols, int main_rows, double* max, int* step) {
    int out = 0;
    *max = -1.0;
    for (int i = 0; i < PLOT_BUF_LEN; i++) {
        DATA_BUF[i] = -1.0;
    }
    for (int i = 0; i < len; i++) {
        if (data[i] > *max) {
            *max = data[i];
        }
    }
    //int out_len = -1;
    *step = 1;
    if (len > main_cols) {
        *step = len/main_cols;
        if (len%main_cols > 0) {
            *step += 1;
        }
        // set each value to the max it can see
        int bin = 0;
        int done = 0;
        while (!done) {
            double bin_max = -1.0;
            for (int i = 0; i < *step; i++) {
                if (bin*(*step)+i >= len) {
                    if (bin_max == -1.0) {
                        out = bin-1;
                    } else {
                        out = bin;
                    }
                    done = 1;
                    break;
                }
                if (data[bin*(*step)+i] > bin_max) {
                    bin_max = data[bin*(*step)+i];
                }
            }
            if (bin_max <= 0.0) {
                DATA_BUF[bin] = -1;
            } else {
                DATA_BUF[bin] = bin_max/(*max) * main_rows;
            }
            bin += 1;
        }
    } else if (len < main_cols) { // Seems to work properly
        *step = main_cols/len;
        if ((*step+1)*(len-1)+1 < main_cols) {
            *step += 1;
        }
        int i = 0;
        while (i < len) {
            DATA_BUF[i*(*step)] = data[i]/(*max)*main_rows;
            i += 1;
        }
        out = *step*(len-1)+1;
        *step *= -1;
    } else { // Should work properly
        for (int i = 0; i < len; i++) {
            DATA_BUF[i] = data[i]/(*max)*main_rows;
        }
        out = len;
    }
    return out;
}

void last2(int start, int step, int main_cols) {
    char PLOT_BUF2[PLOT_BUF_LEN];
    for (int i = 0; i < PLOT_BUF_LEN; i++) {
        PLOT_BUF2[i] = ' ';
    }
    for (int i = 0; i < LEFT_OFFSET-1; i++) {
        PLOT_BUF[i] = ' ';
        PLOT_BUF2[i] = ' ';
    }
    PLOT_BUF[LEFT_OFFSET-1] = '+';
    for (int i = LEFT_OFFSET; i < LEFT_OFFSET+main_cols; i++) {
        PLOT_BUF[i] = '-';
    }
    PLOT_BUF[LEFT_OFFSET+main_cols] = '+';
    PLOT_BUF[LEFT_OFFSET+main_cols+1] = '\0';
    int cumulative = 0;
    while (cumulative < main_cols) {
        if (step < 0 && cumulative > 0) {
            cumulative = ((cumulative/(-step)) + 1) * (-step);
        }
        PLOT_BUF[LEFT_OFFSET+cumulative] = '+';
        int val = 0;
        if (step == 1) {
            val = cumulative+start;
        } else if (step > 1) {
            val = cumulative*step + start;
        } else {
            val = -cumulative/step + start;
        }
        cumulative += sprintf(PLOT_BUF2+cumulative+LEFT_OFFSET, "%d    ", val);
        PLOT_BUF2[LEFT_OFFSET+cumulative] = ' ';
        if (step < 0) {
            //cumulative = ((cumulative/(-step)) + 1) * (-step);
        }
    }
    PLOT_BUF2[LEFT_OFFSET+cumulative] = '\0';
    printf("%s\n", PLOT_BUF);
    printf("%s\n", PLOT_BUF2);
    fflush(stdout);
}

void draw(int rows, int cols, double* data, int start, int len) {
    for (int i = 0; i < PLOT_BUF_LEN; i++) {
        PLOT_BUF[i] = '\0';
    }
    if (cols > PLOT_BUF_LEN) {
        cols = PLOT_BUF_LEN;
    }
    // we fit the data into DATA_BUF
    int main_cols = cols-LEFT_OFFSET-RIGHT_OFFSET;
    int main_rows = rows-5;
    //draw_horiz(main_cols);
    double max;
    int step;
    main_cols = fit_data(data, len, main_cols, main_rows, &max, &step);
    draw_horiz(main_cols);
    int counter = 0;
    for (int r = main_rows-1; r >= 0; r--) {
        double right = -1.0;
        if ((counter++)%5==0 || r==0) {
            right = max*((double)r)/(main_rows-1);
        }
        draw_main(main_cols, r, -1.0, right);
    }
    last2(start, step, main_cols);
}
