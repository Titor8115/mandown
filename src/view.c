#include "view.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <locale.h>
#include <stdio.h>

#include "buffer.h"
#include "mandown.h"

void formatHandler(struct parts *dest, xmlChar *content, int line_fold)
{
  int i, length;

  length = xmlStrlen(content);
  for (i = 0; i < length; i++) {
    getyx(dest->container, dest->curY, dest->curX);

    if (i && i + 1 == length && content[i] == '\n')
      break;

    if ((dest->curX >= dest->width - 1) || content[i] == '\n') {
      if (dest->curY >= dest->height - 1) {
        dest->height += 1;
        wresize(dest->container, dest->height, dest->width);
      }
    }
    else if (dest->curX == 0) {
      wattron(dest->container, A_INVIS);
      wprintw(dest->container, "%*s", line_fold, "");
      wattroff(dest->container, A_INVIS);
    }
    waddch(dest->container, content[i]);
  }
}

void nodeHandler(xmlNode *node, struct parts *dest, int indent)
{
  char *context;
  int line_fold;
  int attr[3];

  xmlNode *curNode;
  line_fold = indent;

  for (curNode = node; curNode != NULL; curNode = curNode->next) {
    context = NULL;
    attr[0] = standard;
    attr[1] = A_NORMAL;
    attr[2] = A_NORMAL;
    if (curNode->type == XML_ELEMENT_NODE) {
      if (STRING_IS("article", curNode->parent->name)) {
        wattrset(dest->container, 0);
        if (STRING_IS("title", curNode->name))
          line_fold = 0;
        else {
          line_fold = FOLDS;
          FORMAT(dest, "\n", line_fold);
        }
      }
      if (STRING_IS("li", curNode->parent->name)) {
        if (!STRING_IS("ul", curNode->name)) {
          FORMAT(dest, "\n", line_fold);
          // if (line_fold >= indent)
          context = "\u00b7 ";
        }
      }

      if (STRING_IS("ul", curNode->name) || STRING_IS("ul", curNode->name)) {  // * unordered list
        line_fold += 3;
      }
      else if (STRING_IS("p", curNode->name)) {  // * paragraph
        // wattrset(dest->container, 0);
      }
      else if (STRING_IS("li", curNode->name)) {  //  * list item
        // line_fold = indent;
      }
      else if (STRING_IS("code", curNode->name)) {  //  * codeblock
        attr[1] = A_CHARTEXT;
        attr[0] = yellow;
        context = ">> ";
      }
      else if (STRING_IS("strong", curNode->name)) {  //  * bold
        attr[1] = A_BOLD;
      }
      else if (STRING_IS("em", curNode->name)) {  //  * italic
        attr[1] = A_ITALIC;
      }
      else if (STRING_IS("u", curNode->name) || STRING_IS("ins", curNode->name)) {  // * underline
        attr[1] = A_UNDERLINE;
      }
      else if (STRING_IS("s", curNode->name) || STRING_IS("del", curNode->name)) {  // * strikethrough
        attr[1] = A_INVIS;
        attr[2] = A_REVERSE;
      }
      else if (STRING_IS("kbd", curNode->name)) {  //  * keyboard input
        attr[1] = A_DIM;
      }
      else if (STRING_IS("a", curNode->name)) {  //  * hyperlink
        attr[1] = A_UNDERLINE;
        attr[0] = blue;
      }
      else if (STRING_IS("h1", curNode->name)) {  //  * header 1 as .SH "NAME"
        line_fold = 0;
        context = "\nNAME";
        attr[1] = A_BOLD;
      }
      else if (STRING_IS("h2", curNode->name)) {  //  * header 2 for other .SH
        line_fold = 0;
        context = "\n";
        attr[1] = A_BOLD;
      }
      else if (STRING_IS("h3", curNode->name)) {  //  * header 3 for .SS
        line_fold = 0;
        context = "\n   ";
        attr[1] = A_BOLD;
      }
      else if (STRING_IS("h4", curNode->name)) {  //  * header 4 as Sub of .SS
        line_fold = 4;
        context = "\nSUB: ";
        attr[1] = A_BOLD;
      }
      else if (STRING_IS("h5", curNode->name)) {  //  * header 5 as Points in Sub
        line_fold = 5;
        context = "\nPT: ";
        attr[1] = A_BOLD;
      }
      else if (STRING_IS("h6", curNode->name)) {  // * header 6 as Sub of Points
        line_fold = 6;
        context = "\nPT SUB: ";
        attr[1] = A_BOLD;
      }
      else {
        if (STRING_IS("img", curNode->name)) {
          context = GET_PROP("alt", curNode);
          attr[1] = A_UNDERLINE;
          attr[0] = blue;
        }
      }

      wattron(dest->container, COLOR_PAIR(attr[0]));
      wattron(dest->container, attr[1]);
      wattron(dest->container, attr[2]);
      if (context != NULL) {
        FORMAT(dest, context, line_fold);
      }
    }
    else if (curNode->type == XML_TEXT_NODE) {
      if (STRING_IS("li", curNode->parent->name)) {
        context = "\n\u00b7 ";
      }
      else if (STRING_IS("h1", curNode->parent->name)) {
        context = "\n";
        line_fold = FOLDS;
      }

      if (context != NULL) {
        FORMAT(dest, context, line_fold);
      }
      if (!STRING_IS("\n", curNode->content)) {
        FORMAT(dest, curNode->content, line_fold);
      }
      // wattrset(dest->container, A_NORMAL);
    }
    nodeHandler(curNode->children, dest, line_fold);
    wattroff(dest->container, COLOR_PAIR(attr[0]));
    wattroff(dest->container, attr[1]);
    wattroff(dest->container, attr[2]);
  }
}

struct parts *
partsNew()
{
  struct parts *ret;
  ret = malloc(sizeof(struct parts));

  if (ret) {
    ret->container = NULL;
    ret->height = 0;
    ret->width = 0;
    ret->curY = 0;
    ret->curX = 0;
  }
  return ret;
}

void partsFree(struct parts *part)
{
  if (!part)
    return;
  delwin(part->container);
  free(part);
}

int view(struct buf *ob, int blocks)
{
  int ymax, xmax, key, curLine, pageHeight;
  struct parts *content, *info;
  xmlDoc *doc;
  xmlNode *rootNode;

  LIBXML_TEST_VERSION;

  //  Render result
  doc = xmlReadMemory((char *)(ob->data), (int)(ob->size), "noname.xml", NULL, XML_PARSE_RECOVER | XML_PARSE_NOBLANKS);
  if (doc == NULL) {
    error("Failed to parse document\n");
    return 1;
  }

  rootNode = xmlDocGetRootElement(doc);
  if (rootNode == NULL) {
    error("empty document\n");
    xmlFreeDoc(doc);
    return 1;
  }

  //  Initialize ncurses
  setlocale(LC_ALL, "");
  initscr();
  cbreak(); /* make getch() process one char at a time */
  noecho(); /* disable output of keyboard typing */

  // keypad(stdscr, TRUE); /* enable arrow keys */
  // curs_set(0); /* disable cursor */

  //  Initialize colors if terminal support
  if (has_colors()) {
    start_color();
    use_default_colors();

    //  todo: 256 color mode
    if (COLORS == 256) {
      init_pair(standard, -1, -1);
      init_pair(red, COLOR_RED, -1);
      init_pair(green, COLOR_GREEN, -1);
      init_pair(yellow, COLOR_YELLOW, -1);
      init_pair(blue, COLOR_BLUE, -1);
      init_pair(magenta, COLOR_MAGENTA, -1);
      init_pair(cyan, COLOR_CYAN, -1);
      init_pair(white, COLOR_WHITE, -1);
    }
    //  8 color mode
    else {
      init_pair(standard, -1, -1);
      init_pair(red, COLOR_RED, -1);
      init_pair(green, COLOR_GREEN, -1);
      init_pair(yellow, COLOR_YELLOW, -1);
      init_pair(blue, COLOR_BLUE, -1);
      init_pair(magenta, COLOR_MAGENTA, -1);
      init_pair(cyan, COLOR_CYAN, -1);
      init_pair(white, COLOR_WHITE, -1);
    }
  }

  getmaxyx(stdscr, ymax, xmax);
  pageHeight = ymax - 1;
  curLine = 0;

  content = partsNew(); /* file content */
  info = partsNew();    /* info content at bottom */

  content->height = blocks;
  content->width = xmax;
  content->container = newpad(content->height, content->width);
  keypad(content->container, TRUE); /* enable arrow keys */

  info->height = 1;
  info->width = xmax;
  info->container = newwin(info->height, info->width, pageHeight, 0);
  scrollok(info->container, TRUE); /* newline implement as auto refresh */

  nodeHandler(rootNode, content, FOLDS);
  xmlFreeDoc(doc);

  prefresh(content->container, curLine, 0, 0, 0, pageHeight, xmax);

  //  Render initial all content sections
  wattrset(info->container, A_REVERSE);
  if (pageHeight >= content->height) {
    wprintw(info->container, "\n Markdown page (ALL) (press q to quit)");
  }
  else {
    wprintw(info->container, "\n Markdown page (TOP) (press q to quit)");
  }

  wrefresh(info->container);

  while ((key = wgetch(content->container)) != 'q') {
    switch (key) {
      case KEY_BACKSPACE:
      case KEY_UP:
        if (curLine > 0)
          curLine--;
        break;
      case ENTER:
      case KEY_DOWN:
        if (curLine + pageHeight < content->height)
          curLine++;
        break;
      default:
        break;
    }
    prefresh(content->container, curLine, 0, 0, 0, pageHeight, xmax);
    if (pageHeight >= content->height) {
      wprintw(info->container, "\n Markdown page (ALL) (press q to quit)");
    }
    else if (curLine <= 0) {
      wprintw(info->container, "\n Markdown page (TOP) (press q to quit)");
    }
    else if (curLine + pageHeight < content->height)
      wprintw(info->container,
              "\n Markdown page (%d%%) (press q to quit)",
              ((curLine + pageHeight) * 100) / content->height);
    else
      wprintw(info->container, "\n Markdown page (END) (press q to quit)");
    wrefresh(info->container);
  }

  //  Clean up
  partsFree(content);
  partsFree(info);
  endwin();
  return 0;
}

// int newlines = 0, Choice = 0, Key = 0;
// for (int i = 0; i < ob->size; i++)
//     if (ob->data[i] == '\n') newlines++;
// int PadHeight = ((ob->size - newlines) / Width + newlines + 1);
// mypad = newpad(PadHeight, Width);
// keypad(content->container, true);
// waddwstr(content->container, c_str());
// refresh();
// int curLine = 0;

/* vim: set filetype=c: */
