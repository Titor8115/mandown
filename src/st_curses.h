#ifndef MDN_ST_CURSES_H
#define MDN_ST_CURSES_H

#if __has_include(<ncurses.h>)
#include <ncurses.h>
#else
#include <ncursesw/ncurses.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define TAB        9
#define ENTER      10
#define CTRL(c)    ((c) & 0x1f)
#define MOUSE_WHEEL_UP   0x00080000
#define MOUSE_WHEEL_DOWN 0x08000000

#ifndef A_ITALIC
#define A_ITALIC NCURSES_BITS(1U, 23)
#endif

#ifdef __cplusplus
}
#endif

#endif /* MDN_ST_CURSES_H */
