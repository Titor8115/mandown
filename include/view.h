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

typedef struct _parts parts;
typedef parts *partsPtr;
struct _parts {
  WINDOW *container;
  int height;
  int width;
  int curY;
  int curX;
  bool resizing;
};

typedef enum {
  Black,
  Red,
  Green,
  Yellow,
  Blue,
  Magenta,
  Cyan,
  White,
} Palette;

#define IS_STRING(string, node) \
  xmlStrEqual((xmlChar *)string, node)

#define GET_PROP(string, node) \
  (char *)xmlGetProp(node, (xmlChar *)string)

#define FORMAT(to, string, folding) \
  formatHandler(to, (xmlChar *)string, folding)

#define GET_SCREEN_SIZE(y ,x) \
  (y = getmaxy(stdscr) - 1, x = getmaxx(stdscr))

int view(struct buf *, int);
void formatHandler(partsPtr, xmlChar *, int);
void nodeHandler(xmlNode *, partsPtr, int); /* set rendering rule for node  */
partsPtr partsNew(int, int, int, int);       /* allocate new WINDOW and its information */
void partsFree(partsPtr);                   /* free Ncurses WINDOW */
