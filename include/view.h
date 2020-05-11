#include <libxml/parser.h>
#include <libxml/tree.h>

#if defined __has_include
#if __has_include(<ncursesw/ncurses.h>)
#include <ncursesw/ncurses.h>
#elif __has_include(<ncurses.h>)
#include <ncurses.h>
#endif
#endif

#include "buffer.h"

#define FOLDS 7
#define ENTER 10

struct parts {
  WINDOW *container;
  int height;
  int width;
  int curY;
  int curX;
};

typedef enum {
  standard,
  red,
  green,
  yellow,
  blue,
  magenta,
  cyan,
  white,
} palette;

#define STRING_IS(string, node) \
  xmlStrEqual((xmlChar *)string, node)

#define GET_PROP(string, node) \
  (char *)xmlGetProp(node, (xmlChar *)string)

#define FORMAT(to, string, indent) \
  formatHandler(to, (xmlChar *)string, indent)

int view(struct buf *, int);
void indentHandler(struct parts *, xmlChar *, int);
void nodeHandler(xmlNode *, struct parts *, int); /* set rendering rule for node  */
struct parts *partsNew();                         /* allocate new WINDOW and its information */
void partsFree(struct parts *);                   /* free Ncurses WINDOW */
