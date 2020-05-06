#include <libxml/parser.h>
#include <libxml/tree.h>
#include <ncursesw/ncurses.h>

#include "buffer.h"

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
  invert,
} palette;

int view(struct buf *, int);
void styleHandler(struct parts *, xmlChar *, int, int);
void nodeHandler(xmlNode *, struct parts *); /* Set rendering rule for node  */
struct parts *partsNew();                    /* Allocate new WINDOW and its information */
void partsFree(struct parts *);              /* Free WINDOW and its information*/
