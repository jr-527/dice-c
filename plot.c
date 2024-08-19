#include "defs.c"
#include <stdio.h>

// This file makes neat ASCII plots.

/**
 * This draws a horizontal line to standard out.
 * 
 * \param main_cols The number of character columns we have to work with, as
 * specified in main.c
 */
void draw_horiz(const int main_cols) {
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

/**
 * This prints one row of the plot window to standard out
 * 
 * \param left The label to put on the left of the row (negative for no label)
 * \param right The label to put on the right of the row (negative for no label)
 */
void draw_main(const int main_cols, const int r, const double left, const double right) {
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
            if (DATA_BUF[i-LEFT_OFFSET] >= r+(2.0/3)) {
                PLOT_BUF[i] = '@';
            } else if (DATA_BUF[i-LEFT_OFFSET] >= r+(1.0/3)) {
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

/**
 * Parses a large array into something short that we can easily plot
 * 
 * \param[in] data The input PMF as an array. Should be positive, sum to 1.
 * \param len Length of data
 * \param main_cols Number of character columns to fit the ASCII art into
 * \param main_rows Number of character rows to fit the ASCII art into
 * \param start Abscissa value of data[0]
 * \param[out] max Location to store the max of data
 * \param[out] step Location to store the step size, as used by last2()
 * \param[out] mean Location to store the expected value of the input PMF
 * \param[out] stdev Location to put the standard deviation of the input PMF
 */
int fit_data(const double* data, const int64_t len, const int main_cols,
             const int main_rows, const int64_t start,
             double* max, int* step, double* mean, double* stdev) {
    int out = 0;
    double mu, s, weight_sum, new_max;
    mu = s = weight_sum = new_max = 0.0;
    for (int64_t i = 0; i < len; i++) {
        double weight = data[i];
        if (weight > new_max) {
            new_max = weight;
        }
        if (weight==0.0) {
            continue;
        }
        int64_t x = i+start;
        weight_sum += weight;
        double old_mu = mu;
        mu += (weight / weight_sum) * (x - old_mu);
        s += weight * (x - old_mu) * (x - mu);
    }
    *max = new_max;
    *mean = mu;
    *stdev = sqrt(s/weight_sum);
    for (int i = 0; i < PLOT_BUF_LEN; i++) {
        DATA_BUF[i] = -1.0;
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
        int64_t i = 0;
        while (i < len) {
            DATA_BUF[i*(*step)] = data[i]/(*max)*main_rows;
            i += 1;
        }
        out = *step*(len-1)+1;
        *step *= -1;
    } else { // Should work properly
        for (int64_t i = 0; i < len; i++) {
            DATA_BUF[i] = data[i]/(*max)*main_rows;
        }
        out = len;
    }
    return out;
}

/**
 * Prints the last two rows of the ASCII art to standard out.
 *
 * \param start leftmost x-label
 * \param step If positive, the number of data points per "bin" (char column) in the plot.
 * If negative, the number of columns per data point.
 * \param main_cols Number of character columns to fit each row into.
 */
void last2(const int64_t start, const int step, const int main_cols) {
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

/** Draws an ASCII plot of the input data to standard out.
 * 
 * \param rows Number of rows (height) the plot must fit into
 * \param cols Number of columns (width) the plot must fit into
 * \param[in] data Array to plot. Should be positive, sum to 1.
 * \param start Abscissa of data[0]
 * \param len Length of data
 */
void draw(const int rows, int cols, const double* data,
          const int64_t start, const int64_t len) {
    for (int i = 0; i < PLOT_BUF_LEN; i++) {
        PLOT_BUF[i] = '\0';
    }
    if (cols > PLOT_BUF_LEN) {
        cols = PLOT_BUF_LEN;
    }
    // we fit the data into DATA_BUF
    int main_cols = cols-LEFT_OFFSET-RIGHT_OFFSET;
    int main_rows = rows-6;
    //draw_horiz(main_cols);
    double max;
    int step;
    double mean, stdev;
    main_cols = fit_data(data, len, main_cols, main_rows, start,
                         &max, &step, &mean, &stdev);
    printf("Average: %.15g, Standard deviation: %.15g\n", mean, stdev);
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
