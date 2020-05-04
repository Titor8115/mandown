#include <libxml/parser.h>
#include <libxml/tree.h>
#include <ncursesw/ncurses.h>

#include "buffer.h"

#define TABSIZE 8

struct parts {
  WINDOW *container;
  int height;
  int width;
  int curY;
  int curX;
};

typedef enum {
  //   BLK,
  red = 1,
  green,
  yellow,
  blue,
  magenta,
  cyan,
  white,
} palette;

int view(struct buf *, int);

void formatHandler(struct parts *, xmlChar *, int, bool);

void nodeHandler(xmlNode *, struct parts *);

struct parts *partsNew();

void partsFree(struct parts *);
