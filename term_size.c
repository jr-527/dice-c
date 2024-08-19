#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
/**
 * Finds the size of the user's terminal.
 * \param[out] rows Location to store the number of rows (height)
 * \param[out] cols Location to store the number of columns (width)
 */
void get_term_size(int* rows, int* cols) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    *cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    *rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
}
#else
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>

/**
 * Finds the size of the user's terminal.
 * \param[out] rows Location to store the number of rows (height)
 * \param[out] cols Location to store the number of columns (width)
 */
void get_term_size(int* rows, int* cols) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    *rows = w.ws_row;
    *cols = w.ws_col;
}
#endif
