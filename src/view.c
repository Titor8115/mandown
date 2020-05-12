#include "view.h"

// #include <libxml/parser.h>
#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>
// #include <libxml/tree.h>
#include <locale.h>
#include <stdio.h>

#include "buffer.h"
#include "mandown.h"

void formatHandler(partsPtr dest, xmlChar *content, int line_fold)
{
  int i, length;

  length = xmlStrlen(content);
  for (i = 0; i < length; i++) {
    if (dest->resizing) return;

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

void nodeHandler(xmlNode *node, partsPtr dest, int indent)
{
  char *context;
  int line_fold;
  int attr[3];

  xmlBufferPtr buffer;
  xmlNode *curNode;

  if (dest->resizing) return;

  line_fold = indent;
  for (curNode = node; curNode != NULL; curNode = curNode->next) {
    if (dest->resizing) return;

    context = NULL;
    attr[0] = Black;
    attr[1] = A_NORMAL;
    attr[2] = A_NORMAL;
    if (curNode->type == XML_ELEMENT_NODE) {
      if (IS_STRING("article", curNode->parent->name)) {
        wattrset(dest->container, 0);
        if (IS_STRING("title", curNode->name))
          line_fold = 0;
        else {
          line_fold = FOLDS;
          FORMAT(dest, "\n", line_fold);
        }
      }
      // else if (IS_STRING("li", curNode->parent->name)) {
      //   if (!IS_STRING("ul", curNode->name)) {
      //     // FORMAT(dest, "\n", line_fold);
      //     // if (line_fold >= indent)
      //     // context = "\u00b7 ";
      //   }
      // }

      if (IS_STRING("ul", curNode->name) || IS_STRING("ul", curNode->name)) {  // * unordered list
        line_fold += 3;
      }
      else if (IS_STRING("p", curNode->name)) {  // * paragraph
        // wattrset(dest->container, 0);
      }
      else if (IS_STRING("li", curNode->name)) {  //  * list item
        context = "\n\u00b7 ";

        // line_fold = indent;
      }
      else if (IS_STRING("code", curNode->name)) {  //  * codeblock
        buffer = xmlBufferCreate();
        xmlNodeDump(buffer, curNode->children->doc, curNode->children, 0, 0);
        attr[1] = A_CHARTEXT;
        attr[0] = Yellow;
        // line_fold = indent + 3;
        wattron(dest->container, COLOR_PAIR(attr[0]));
        wattron(dest->container, attr[1]);
        wattron(dest->container, attr[2]);
        FORMAT(dest, (char *)xmlBufferContent(buffer), line_fold);
        wattroff(dest->container, COLOR_PAIR(attr[0]));
        wattroff(dest->container, attr[1]);
        wattroff(dest->container, attr[2]);
        xmlFree(buffer);
        continue;
      }
      else if (IS_STRING("strong", curNode->name)) {  //  * bold
        attr[1] = A_BOLD;
      }
      else if (IS_STRING("em", curNode->name)) {  //  * italic
        attr[1] = A_ITALIC;
      }
      else if (IS_STRING("u", curNode->name) || IS_STRING("ins", curNode->name)) {  // * underline
        attr[1] = A_UNDERLINE;
      }
      else if (IS_STRING("s", curNode->name) || IS_STRING("del", curNode->name)) {  // * strikethrough
        attr[1] = A_INVIS;
        attr[2] = A_REVERSE;
      }
      else if (IS_STRING("kbd", curNode->name)) {  //  * keyboard input
        attr[1] = A_DIM;
      }
      else if (IS_STRING("a", curNode->name)) {  //  * hyperlink
        attr[1] = A_UNDERLINE;
        attr[0] = Blue;
      }
      else if (IS_STRING("h1", curNode->name)) {  //  * header 1 as .SH "NAME"
        line_fold = 0;
        context = "\nNAME";
        attr[1] = A_BOLD;
      }
      else if (IS_STRING("h2", curNode->name)) {  //  * header 2 for other .SH
        line_fold = 0;
        context = "\n";
        attr[1] = A_BOLD;
      }
      else if (IS_STRING("h3", curNode->name)) {  //  * header 3 for .SS
        line_fold = 0;
        context = "\n   ";
        attr[1] = A_BOLD;
      }
      else if (IS_STRING("h4", curNode->name)) {  //  * header 4 as Sub of .SS
        line_fold = 4;
        context = "\nSUB: ";
        attr[1] = A_BOLD;
      }
      else if (IS_STRING("h5", curNode->name)) {  //  * header 5 as Points in Sub
        line_fold = 5;
        context = "\nPT: ";
        attr[1] = A_BOLD;
      }
      else if (IS_STRING("h6", curNode->name)) {  // * header 6 as Sub of Points
        line_fold = 6;
        context = "\nPT SUB: ";
        attr[1] = A_BOLD;
      }
      else {
        if (IS_STRING("img", curNode->name)) {
          context = GET_PROP("alt", curNode);
          attr[1] = A_UNDERLINE;
          attr[0] = Blue;
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
      if (IS_STRING("li", curNode->parent->name)) {
        // context = "\n\u00b7 ";
      }
      else if (IS_STRING("h1", curNode->parent->name)) {
        context = "\n";
        line_fold = FOLDS;
      }

      if (context != NULL) {
        FORMAT(dest, context, line_fold);
      }
      if (!IS_STRING("\n", curNode->content)) {
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

partsPtr
partsNew(int height, int width, int curY, int curX)
{
  partsPtr ret;
  ret = malloc(sizeof(parts));

  if (ret) {
    ret->container = NULL;
    ret->height = height;
    ret->width = width;
    ret->curY = curY;
    ret->curX = curX;
    ret->resizing = false;
  }
  return ret;
}

void partsFree(partsPtr part)
{
  if (!part)
    return;
  delwin(part->container);
  free(part);
}

int view(struct buf *ob, int blocks)
{
  int ymax, xmax, key, curLine;

  partsPtr page, status;
  xmlDoc *doc;
  xmlNode *rootNode;

  LIBXML_TEST_VERSION;

  //  Render result
  doc = htmlReadMemory((char *)(ob->data), (int)(ob->size),
                       "input markdown", NULL,
                       HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
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
  // cbreak(); /* make getch() process one char at a time */
  halfdelay(1);
  noecho(); /* disable output of keyboard typing */

  curs_set(0); /* disable cursor */

  //  Initialize colors if terminal support
  if (has_colors()) {
    start_color();
    use_default_colors();

    //  todo: 256 color mode
    if (COLORS == 256) {
      init_pair(Black, -1, -1);
      init_pair(Red, COLOR_RED, -1);
      init_pair(Green, COLOR_GREEN, -1);
      init_pair(Yellow, COLOR_YELLOW, -1);
      init_pair(Blue, COLOR_BLUE, -1);
      init_pair(Magenta, COLOR_MAGENTA, -1);
      init_pair(Cyan, COLOR_CYAN, -1);
      init_pair(White, COLOR_WHITE, -1);
    }
    //  8 color mode
    else {
      init_pair(Black, -1, -1);
      init_pair(Red, COLOR_RED, -1);
      init_pair(Green, COLOR_GREEN, -1);
      init_pair(Yellow, COLOR_YELLOW, -1);
      init_pair(Blue, COLOR_BLUE, -1);
      init_pair(Magenta, COLOR_MAGENTA, -1);
      init_pair(Cyan, COLOR_CYAN, -1);
      init_pair(White, COLOR_WHITE, -1);
    }
  }
  GET_SCREEN_SIZE(ymax, xmax);
  curLine = 0;
  page = partsNew(blocks, xmax, 0, 0); /* file page */
  page->container = newpad(page->height, page->width);
  keypad(page->container, TRUE); /* enable arrow keys */

  status = partsNew(1, xmax, ymax, 0); /* status page at bottom */
  status->container = newwin(status->height, status->width, status->curY, status->curX);
  scrollok(status->container, TRUE); /* newline implement as auto refresh */

  refresh();
  wattrset(status->container, A_REVERSE);
  nodeHandler(rootNode, page, FOLDS);

  while ((key = wgetch(page->container)) != 'q') {
    switch (key) {
      case 'j':
      case ENTER:
      case KEY_DOWN:
        if (curLine + ymax < page->height)
          curLine++;
        break;
      case KEY_RESIZE:
        halfdelay(2);
        page->resizing = true;
        GET_SCREEN_SIZE(ymax, xmax);
        break;
      case 'k':
      case KEY_BACKSPACE:
      case KEY_UP:
        if (curLine > 0)
          curLine--;
        break;
      case -1:
        cbreak();
        page->resizing = false;
        GET_SCREEN_SIZE(ymax, xmax);
        if (page->width != xmax) {
          page->width = (page->width > xmax) ? page->width + page->width / 5 : page->width - page->width / 5;
          werase(page->container);
          wresize(page->container, page->height, xmax);
          nodeHandler(rootNode, page, FOLDS);
        }
        if (status->curY != ymax)
          mvwin(status->container, ymax, 0);
        break;
      default:
        page->resizing = false;
        break;
    }
    if (!page->resizing) {
      prefresh(page->container, curLine, 0, 0, 0, ymax, xmax);
    }

    if (ymax >= page->height) {
      wprintw(status->container, "\n Markdown page (ALL) (press q to quit)");
    }
    else if (curLine <= 0) {
      wprintw(status->container, "\n Markdown page (TOP) (press q to quit)");
    }
    else if (curLine + ymax < page->height)
      wprintw(status->container,
              "\n Markdown page (%d%%) (press q to quit)",
              ((curLine + ymax) * 100) / page->height);
    else
      wprintw(status->container, "\n Markdown page (END) (press q to quit)");
    wrefresh(status->container);
  }

  //  Clean up
  xmlFreeDoc(doc);
  partsFree(page);
  partsFree(status);
  endwin();
  return 0;
}

/* vim: set filetype=c: */
