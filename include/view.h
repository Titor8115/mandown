#include <libxml/parser.h>
#include <libxml/tree.h>
#include <ncurses.h>

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
  black,
  red,
  green,
  yellow,
  blue,
  magenta,
  cyan,
  white,
} palette;

#define STRING_IS(string, name) \
  xmlStrEqual((xmlChar *)string, name)

#define FORMAT(to, string, indent) \
  formatHandler(to, (xmlChar *)string, indent)

int view(struct buf *, int);
void indentHandler(struct parts *, xmlChar *, int);
void nodeHandler(xmlNode *, struct parts *, int); /* set rendering rule for node  */
struct parts *partsNew();                    /* allocate new WINDOW and its information */
void partsFree(struct parts *);              /* free Ncurses WINDOW */
