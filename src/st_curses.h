#ifdef HAS_NCURSES_H
#include <ncurses.h>
#else
#include <ncursesw/ncurses.h>
#endif

#define TAB   9
#define ENTER 10

#ifndef A_ITALIC
#define A_ITALIC NCURSES_BITS(1U, 23)
#endif

#define get_display_size(y, x) (y = getmaxy(stdscr) - 1, x = getmaxx(stdscr))
