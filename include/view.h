#ifndef MDN_VIEW_H
#define MDN_VIEW_H

#if defined __has_include
#if __has_include(<ncursesw/ncurses.h>)
#include <ncursesw/ncurses.h>
#elif __has_include(<ncurses.h>)
#include <ncurses.h>
#endif
#if __has_include(<libxml/HTMLparser.h>)
#include <libxml/HTMLparser.h>
#elif __has_include(<libxml2/libxml/HTMLparser.h>)
#include <libxml2/libxml/HTMLparser.h>
#endif
#if __has_include(<libxml/tree.h>)
#include <libxml/tree.h>
#elif __has_include(<libxml2/libxml/tree.h>)
#include <libxml2/libxml/tree.h>
#endif
#if __has_include(<libxml/xmlerror.h>)
#include <libxml/xmlerror.h>
#elif __has_include(<libxml2/libxml/xmlerror.h>)
#include <libxml2/libxml/xmlerror.h>
#endif
#else
#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>
#include <libxml/xmlerror.h>
#include <ncursesw/ncurses.h>
#endif

#include <stdbool.h>
#include <stdint.h>

#include "buffer.h"
#include "mandown.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ENTER 10

typedef struct contents Contents;
struct contents {
  Contents *pre;
  Contents *next;
  uint8_t * string;
  int       length;
  int       color;
  int       firAttr;
  int       secAttr;
  int       fold;
  int       newline;
  bool      resetAttr;
};

struct parts {
  WINDOW *ctnr;
  int     height;
  int     width;
  int     curY;
  int     curX;
  int     resize_key;
  bool    resized;
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

#define IS_STRING(string, node) \
  xmlStrEqual((xmlChar *)string, (xmlChar *)node)

#define GET_PROP(node, string) \
  xmlGetProp((xmlNodePtr)node, (xmlChar *)string)

#define ARRANGE(parts, content) \
  arrange_content(parts, content)

#define GET_SCREEN_SIZE(y, x) \
  (y = getmaxy(stdscr) - 1, x = getmaxx(stdscr))

#define TOG_ATTR(win, content, toggle) \
  tog_attr(win, (bool)content->resetAttr, (int)content->color, (int)content->firAttr, (int)content->secAttr, (bool)toggle)

extern int view(const Config *, struct buf *, int);
// void          formatHandler(struct text *, struct parts *);
// struct text * nodeHandler(xmlNode *, int);  /* set rendering rule for node  */
// struct parts *partsNew(int, int, int, int); /* allocate new WINDOW and its information */
// void          partsFree(struct parts *);    /* free Ncurses WINDOW */

#ifdef __cplusplus
}
#endif

#endif /* MDN_VIEW_H */
