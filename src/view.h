#ifndef MDN_VIEW_H
#define MDN_VIEW_H

#include "config.h"
#include "st_curses.h"
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
  FRAME_NONE = 0,
  FRAME_HELP,
  FRAME_STATS,
  FRAME_PAGE,
  FRAME_PROBE,
};

enum node_t {
  N_PLAIN = 0,
  N_EM,
  N_BOLD,
  N_INS,
  N_DEL,
  N_PRE,
  N_KBD,

  N_HEADING,
  N_HREF
};

struct frame {
  WINDOW *win;
  attr_t  attr;
  int     frame_type;
  int     height;
  int     width;
  int     beg_y;
  int     beg_x;
  int     cur_y;
  int     cur_x;
};

struct frame_main {
  struct frame* pad;
  struct frame* bar;
  int     height;
  int     width;
  int     cur_y;
  int     cur_x;
};

#ifdef __cplusplus
}
#endif

#endif /* MDN_VIEW_H */
