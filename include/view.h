#ifndef MDN_VIEW_H
#define MDN_VIEW_H

#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>
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

typedef struct contents Content;
struct contents {
  struct buf *string;
  Content *   next;
  int         fold;
  int         newline;
  int         color;
  int         firAttr;
  int         secAttr;
  bool        togAttr;
  bool        formated;
};

typedef struct parts Part;
struct parts {
  WINDOW *ctnr;
  int     height;
  int     width;
  int     curY;
  int     curX;
};

typedef enum {
  Standard,
  Red,
  Green,
  Yellow,
  Blue,
  Magenta,
  Cyan,
  White,
} Palette;

#define IS_NODE(string, node) \
  xmlStrEqual((xmlChar *)string, (xmlChar *)node)

#define GET_PROP(node, string) \
  xmlGetProp((xmlNodePtr)node, (xmlChar *)string)

#define ARRANGE(parts, content) \
  render_content(parts, content)

#define GET_SCREEN_SIZE(y, x) \
  (y = getmaxy(stdscr) - 1, x = getmaxx(stdscr))

#define TOG_ATTR(win, toggle, content) \
  toggle_attr(win, toggle, content->color, content->firAttr, content->secAttr)

#define COPY_ATTR(dest, source) \
  (dest->color = source->color, dest->firAttr = source->firAttr, dest->secAttr = source->secAttr)

extern int view(const Config *, struct buf *, int);
// void          formatHandler(struct text *, struct parts *);
// struct text * nodeHandler(xmlNode *, int);  /* set rendering rule for node  */
// struct parts *partsNew(int, int, int, int); /* allocate new WINDOW and its information */
// void          partsFree(struct parts *);    /* free Ncurses WINDOW */

#ifdef __cplusplus
}
#endif

#endif /* MDN_VIEW_H */
