#ifndef MDN_VIEW_H
#define MDN_VIEW_H

#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>
#include <libxml/xmlstring.h>
#include <libxml/xmlerror.h>

#ifdef HAS_NCURSES_H
#include <ncurses.h>
#else
#include <ncursesw/ncurses.h>
#endif

#include "buffer.h"
#include "mandown.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ENTER 10
#ifndef A_ITALIC
#define A_ITALIC NCURSES_BITS(1U, 23)
#endif

#define cmp_xml(str, node)          xmlStrEqual((xmlChar *)str, (xmlChar *)node)
#define get_prop(node, str)         xmlGetProp((xmlNode *)node, (xmlChar *)str)
#define get_stdscr_size(y, x)       (y = getmaxy(stdscr) - 1, x = getmaxx(stdscr))
#define update_size(part, y, x)     ((part)->height = y, (part)->width = x)
#define get_frame_size(part, y, x)  (y = getmaxy((part)->win), x = getmaxx((part)->win))

typedef struct content Content;
struct content {
  struct buf *buf;
  Content *   next;
  int         fold;
  attr_t      attr;
  uint8_t     prop;
};

typedef struct frame Frame;
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

struct url {
  struct buf * title;
  struct buf * link;
  struct part *ctnr;
};

extern int view(const Config *, struct buf *, int);
// void          formatHandler(struct text *, struct parts *);
// struct text * nodeHandler(xmlNode *, int);  /* set rendering rule for node  */
// struct parts *partsNew(int, int, int, int); /* allocate new WINDOW and its information */
// void          partsFree(struct parts *);    /* free Ncurses WINDOW */

#ifdef __cplusplus
}
#endif

#endif /* MDN_VIEW_H */
