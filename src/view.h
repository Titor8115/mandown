#ifndef MDN_VIEW_H
#define MDN_VIEW_H

#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>
#include <libxml/xmlstring.h>
#include <libxml/xmlerror.h>

#include "st_curses.h"
#include "mandown.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WHEEL_UP 0x00080000
#define WHEEL_DOWN 0x08000000

#define cmp_xml(str, node)          xmlStrEqual((xmlChar *)str, (xmlChar *)node)
#define get_prop(node, str)         xmlGetProp((xmlNode *)node, (xmlChar *)str)
#define update_size(part, y, x)     ((part)->height = y, (part)->width = x)
#define get_frame_size(part, y, x)  (y = getmaxy((part)->win), x = getmaxx((part)->win))

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

typedef enum {
  N_PLAIN = 0,
  N_EM,
  N_BOLD,
  N_INS,
  N_DEL,
  N_PRE,
  N_KBD,

  N_HEADING,
  N_HREF
} node_t;

struct content {
  struct buf *    buf;
  struct content *next;
  int             fold;
  attr_t          attr;
  uint8_t         prop;
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

// void          formatHandler(struct text *, struct parts *);
// struct text * nodeHandler(xmlNode *, int);  /* set rendering rule for node  */
// struct parts *partsNew(int, int, int, int); /* allocate new WINDOW and its information */
// void          partsFree(struct parts *);    /* free Ncurses WINDOW */

#ifdef __cplusplus
}
#endif

#endif /* MDN_VIEW_H */
