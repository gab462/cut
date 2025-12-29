#ifndef INCLUDE_TERM_H
#define INCLUDE_TERM_H

#include <termios.h>
#include <sys/ioctl.h>

static inline
void
term_set_raw(void)
{
    struct termios cfg;

    tcgetattr(STDIN_FILENO, &cfg);
    cfg.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &cfg);
}

static inline
void
term_set_canon(void)
{
    struct termios cfg;

    tcgetattr(STDIN_FILENO, &cfg);
    cfg.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &cfg);
}

static inline
int
term_width(void)
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    return(w.ws_col);
}

static inline
int
term_height(void)
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    return(w.ws_row);
}

#endif
