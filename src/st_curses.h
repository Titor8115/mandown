// Copyright (C) 2019 Tianze Han
// 
// This file is part of Mandown.
// 
// Mandown is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Mandown is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Mandown.  If not, see <http://www.gnu.org/licenses/>.

#ifndef MDN_ST_CURSES_H
#define MDN_ST_CURSES_H

/* Makefile will define WIDE_NCURSES during compile if libncursesw is detected */
#ifdef WIDE_CURSES
#include <ncursesw/ncurses.h>
#else
#include <ncurses.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define ALT_ESC    27
#define TAB        9
#define ENTER      10
#define CONTROL    0x1f
#define CTRL(c)    ((c) & CONTROL)
/*#define MOUSE_WHEEL_UP   0x00080000*/
/*#define MOUSE_WHEEL_DOWN 0x08000000*/

/*#ifndef A_ITALIC*/
/*#define A_ITALIC NCURSES_BITS(1U, 23)*/
/*#endif*/

#ifdef __cplusplus
}
#endif

#endif /* MDN_ST_CURSES_H */
