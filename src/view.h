/**
 * Copyright (C) 2019 Tianze Han
 * 
 * This file is part of Mandown.
 * 
 * Mandown is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Mandown is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Mandown.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MDN_VIEW_H
#define MDN_VIEW_H

#include "dom.h"

#ifdef __cplusplus
extern "C" {
#endif

enum color_set {
  C_DEFAULT = 0,
  C_RED,
  C_GREEN,
  C_YELLOW,
  C_BLUE,
  C_MAGENTA,
  C_CYAN,

  C_MAX,
};

enum frame_type {
  FRAME_WIN = 0,
  FRAME_SWIN,
  FRAME_PAD,
  FRAME_SPAD
};

enum node_t {
  N_PLAIN = 0,
  N_EM,
  N_BOLD,
  N_INS,
  N_DEL,
  N_PRE,
  N_CODE,
  N_KBD,

  N_HEAD,
  N_HREF
};

struct frame {
  WINDOW *win;
  int     type;
  int     max_y;
  int     max_x;
  int     beg_y;
  int     beg_x;
  int     cur_y;
  int     cur_x;
};

int view(const struct buf *, int);

#ifdef __cplusplus
}
#endif

#endif /* MDN_VIEW_H */
