#include <libxml/parser.h>
#include <libxml/tree.h>
#include <ncursesw/ncurses.h>

#include "buffer.h"

#define ENTER 10

struct parts {
  WINDOW *container;
  int height;
  int width;
  int curY;
  int curX;
};

typedef enum {
  black,
  red,
  green,
  yellow,
  blue,
  magenta,
  cyan,
  white,
} palette;

int view(struct buf *, int);
void styleHandler(struct parts *, xmlChar *, int);
void nodeHandler(xmlNode *, struct parts *); /* set rendering rule for node  */
struct parts *partsNew();                    /* allocate new WINDOW and its information */
void partsFree(struct parts *);              /* free Ncurses WINDOW */
