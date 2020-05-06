#include "view.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <locale.h>
#include <ncursesw/ncurses.h>
#include <stdio.h>

#include "buffer.h"
#include "mandown.h"

void styleHandler(struct parts *dest, xmlChar *content, int indentChar, int color)
{
  int i, length;
  length = xmlStrlen(content);
  wattron(dest->container, COLOR_PAIR(color));

  for (i = 0; i < length; i++) {

    getyx(dest->container, dest->curY, dest->curX);

    if ((dest->curX >= dest->width - 1) || content[i] == '\n') {
      if (dest->curY >= dest->height - 1) {
        dest->height += 1;
        wresize(dest->container, dest->height, dest->width);
      }
      wprintw(dest->container, "\0%c", indentChar);
    }
    else if (dest->curX == 0) {
      wprintw(dest->container, "%c", indentChar);
    }
    waddch(dest->container, content[i]);
  }
  wattrset(dest->container, 0);
}

void nodeHandler(xmlNode *node, struct parts *dest)
{
  xmlNode *curNode;
  int indentChar, color;

  for (curNode = node; curNode != NULL; curNode = curNode->next) {
    indentChar = 0;
    color = standard;

    if (curNode->type == XML_ELEMENT_NODE) {
      if (xmlStrEqual((xmlChar *)"title", curNode->name)) {
      }
      else if (xmlStrEqual((xmlChar *)"p", curNode->name)) {
        if ((curNode->parent == NULL) || xmlStrEqual((xmlChar *)"div", curNode->parent->name))
          styleHandler(dest, (xmlChar *)"\n", indentChar, color);
      }
      else if (xmlStrEqual((xmlChar *)"code", curNode->name)) {
      }

      else if (xmlStrEqual((xmlChar *)"li", curNode->name)) {
        if (xmlStrlen(curNode->children->content) < 1)
          styleHandler(dest, (xmlChar *)"\n\t\u2022 ", indentChar, color);
      }
      else {
        wattron(dest->container, A_BOLD);
        if (xmlStrEqual((xmlChar *)"h1", curNode->name)) {
          wprintw(dest->container, "\n\nNAME\n\t");
        }
        else if (xmlStrEqual((xmlChar *)"h2", curNode->name)) {
          wprintw(dest->container, "\n\n");
        }
        else if (xmlStrEqual((xmlChar *)"h3", curNode->name)) {
          wprintw(dest->container, "\n\n   ");
        }
        else if (xmlStrEqual((xmlChar *)"h4", curNode->name)) {
          wprintw(dest->container, "\n\n      SECTION: ");
        }
        else if (xmlStrEqual((xmlChar *)"h5", curNode->name)) {
          wprintw(dest->container, "\n\n         SUB SECTION: ");
        }
        else if (xmlStrEqual((xmlChar *)"h6", curNode->name)) {
          wprintw(dest->container, "\n\n            POINT: ");
        }
        wattroff(dest->container, A_BOLD);
      }
    }
    else if (curNode->type == XML_TEXT_NODE) {
      if (xmlStrEqual((xmlChar *)"title", curNode->parent->name)) {
        styleHandler(dest, curNode->content, indentChar, color);
      }
      else if (xmlStrEqual((xmlChar *)"p", curNode->parent->name)) {
        indentChar = '\t';
        styleHandler(dest, curNode->content, indentChar, color);
      }
      else if (xmlStrEqual((xmlChar *)"code", curNode->parent->name)) {
        indentChar = '\t';
        color = yellow;

        // wattron(dest->container, COLOR_PAIR(invert));
        // wbkgdset(dest->container, COLOR_PAIR(invert));
        styleHandler(dest, curNode->content, indentChar, color);
        // wbkgdset(dest->container, COLOR_PAIR(standard));
      }
      else if (xmlStrEqual((xmlChar *)"li", curNode->parent->name)) {
        styleHandler(dest, (xmlChar *)"\n\t    \u2022 ", indentChar, color);
        styleHandler(dest, curNode->content, indentChar, color);
      }
      else if (xmlStrEqual((xmlChar *)"strong", curNode->parent->name)) {
        styleHandler(dest, curNode->content, indentChar, color);
      }
      else if (xmlStrEqual((xmlChar *)"em", curNode->parent->name)) {
        styleHandler(dest, curNode->content, indentChar, color);
      }
      else {
        wattron(dest->container, A_BOLD);
        wprintw(dest->container, "%s", curNode->content);
        wattroff(dest->container, A_BOLD);
      }
      // wattrset(dest->container, 0);
    }
    nodeHandler(curNode->children, dest);
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

  doc = xmlReadMemory((char *)(ob->data), (int)(ob->size), "noname.xml", NULL, XML_PARSE_NOBLANKS);
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

  /* Initialize ncurses */
  setlocale(LC_CTYPE, "");
  initscr();
  // keypad(stdscr, TRUE); /* enable arrow keys */
  // curs_set(0); /* disable cursor */
  cbreak(); /* make getch() process one char at a time */
  noecho(); /* disable output of keyboard typing */
  // idlok(stdscr, TRUE);  /* allow use of insert/delete line */

  /* Initialize all colors if terminal support color */
  if (has_colors()) {
    start_color();
    use_default_colors();
    init_pair(standard, COLOR_WHITE, COLOR_BLACK);
    init_pair(red, COLOR_RED, -1);
    init_pair(green, COLOR_GREEN, -1);
    init_pair(yellow, COLOR_YELLOW, -1);
    init_pair(blue, COLOR_BLUE, -1);
    init_pair(magenta, COLOR_MAGENTA, -1);
    init_pair(cyan, COLOR_CYAN, -1);
    init_pair(invert, COLOR_BLACK, COLOR_WHITE);
  }

  getmaxyx(stdscr, ymax, xmax);
  pageHeight = ymax - 1;

  content = partsNew();
  content->height = blocks;
  content->width = xmax;
  content->container = newpad(content->height, content->width);
  keypad(content->container, TRUE); /* enable arrow keys */

  info = partsNew();
  info->height = 1;
  info->width = xmax;
  info->container = newwin(info->height, info->width, pageHeight, 0);
  scrollok(info->container, TRUE);

  /* Render the result */
  nodeHandler(xmlFirstElementChild(rootNode), content);

  curLine = 0;
  prefresh(content->container, curLine, 0, 0, 0, pageHeight, xmax);
  wattron(info->container, COLOR_PAIR(invert));
  if (pageHeight >= content->height) {
    wprintw(info->container, "\n Markdown page (ALL) (press q to quit)");
  }
  else {
    wprintw(info->container, "\n Markdown page (TOP) (press q to quit)");
  }

  wrefresh(info->container);
  // wgetch(content->container);

  while ((key = wgetch(content->container)) != 'q') {
    switch (key) {
      case KEY_UP:
        if (curLine > 0)
          curLine--;
        break;
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

  /* Clean up */
  xmlFreeDoc(doc);
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
