#include "view.h"

#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "buffer.h"

struct parts *
partsNew(int height, int width, int curY, int curX)
{
  struct parts *ret;
  ret = malloc(sizeof *ret);

  if (ret) {
    ret->ctnr = NULL;
    ret->height = height;
    ret->width = width;
    ret->curY = curY;
    ret->curX = curX;
    ret->resize_key = FALSE;
  }
  return ret;
}

void partsFree(struct parts *part)
{
  if (!part)
    return;
  if (part->ctnr != NULL)
    delwin(part->ctnr);
  free(part);
}

Contents *
contentsNew(int fold)
{
  Contents *ret;
  ret = malloc(sizeof *ret);

  if (ret) {
    ret->string = 0;
    ret->length = 0;
    ret->color = Standard;
    ret->firAttr = A_NORMAL;
    ret->secAttr = A_NORMAL;
    ret->fold = fold;
    ret->newline = 0;
    ret->resetAttr = FALSE;
  }
  return ret;
}

// void contentsUpdate(Contents *content, )
// {
//   if (!content)
//     return;

//   content->string = NULL;
//   content->length = 0;
//   content->color = Standard;
//   content->firAttr = content->secAttr = A_NORMAL;
//   // content->fold = fold;
//   content->newline = 0;
//   content->resetAttr = FALSE;
// }

void contentsReset(Contents *content)
{
  if (!content)
    return;

  content->string = NULL;
  content->length = 0;
  content->color = Standard;
  content->firAttr = A_NORMAL;
  content->secAttr = A_NORMAL;
  // content->fold = fold;
  content->newline = 0;
  content->resetAttr = FALSE;
}

void contentsFree(Contents *content)
{
  if (!content)
    return;
  free(content);
}

void tog_attr(WINDOW *dest, bool reset, int color, int firAttr, int secAttr, bool toggle)
{
  if (reset)
    wattrset(dest, 0);

  if (toggle) {
    wattron(dest, COLOR_PAIR(color));
    wattron(dest, firAttr);
    wattron(dest, secAttr);
  }
  else {
    wattroff(dest, COLOR_PAIR(color));
    wattroff(dest, firAttr);
    wattroff(dest, secAttr);
  }
}
void arrange_content(struct parts *dest, Contents *content)
{
  bool newline = FALSE;
  int  length;
  int  i = 0;
  // int        width = dest->width - 1;
  const char toc[] = " ";
  char *     line = NULL;

  for (i; i < content->newline; i++)
    waddch(dest->ctnr, '\n');

  if (content->string == NULL)
    return;
  if (dest->resize_key)
    return;

  // if (content->length + content->fold >= dest->width - 1) {
  //   waddstr(dest->ctnr, (char *)content->string);
  //   return;
  // }

  line = strtok((char *)content->string, toc);

  while (line != NULL) {
    if (dest->resize_key)
      break;

    length = strlen(line);
    getyx(dest->ctnr, dest->curY, dest->curX);

    if (dest->resize_key)
      break;
    if (dest->curY >= dest->height - 1) {
      dest->height += 1;
      wresize(dest->ctnr, dest->height, dest->width);
    }
    if (dest->curX + length >= dest->width - 1) {
      dest->height += 1;
      wresize(dest->ctnr, dest->height, dest->width);
      waddstr(dest->ctnr, "\n");
    }
    getyx(dest->ctnr, dest->curY, dest->curX);

    if (dest->curX == 0) {
      tog_attr(dest->ctnr, FALSE, content->color, content->firAttr, content->secAttr, FALSE);
      wprintw(dest->ctnr, "%*s", content->fold, "");
      tog_attr(dest->ctnr, FALSE, content->color, content->firAttr, content->secAttr, TRUE);
    }

    waddstr(dest->ctnr, line);
    waddch(dest->ctnr, ' ');

    line = strtok(NULL, toc);
  }
  return;
  // length = xmlStrlen(content);
  // for (i = 0; i < length; i++) {
  //   if (dest->resize_key) return;

  //   getyx(dest->ctnr, dest->curY, dest->curX);

  //   if (i && i + 1 == length && content[i] == '\n')
  //     break;

  //   if ((dest->curX >= dest->width - 1) || content[i] == '\n') {
  //     if (dest->curY >= dest->height - 1) {
  //       dest->height += 1;
  //       wresize(dest->ctnr, dest->height, dest->width);
  //     }
  //   }
  //   else if (dest->curX == 0) {
  //     wattron(dest->ctnr, A_INVIS);
  //     wprintw(dest->ctnr, "%*s", line_fold, "");
  //     wattroff(dest->ctnr, A_INVIS);
  //   }
  //   waddch(dest->ctnr, content[i]);
  // }
}

void get_content(xmlNode *node, struct parts *dest, int line_fold)
{
  // size_t       length;
  bool           isCode = FALSE;
  Contents *     piece;
  xmlBufferPtr   buffer;
  xmlNode *      curNode;
  const xmlChar *ret;
  piece = contentsNew(line_fold);
  if (dest->resize_key)
    return;

  for (curNode = node; curNode != NULL; curNode = curNode->next) {
    if (dest->resize_key)
      break;
    if (curNode->type == XML_ELEMENT_NODE) {
      if (IS_STRING("article", curNode->parent->name)) {
        piece->resetAttr = TRUE;
        piece->fold = line_fold;
        piece->newline = 1;
      }
      if (IS_STRING("code", curNode->parent->name)) {
      }
      if (IS_STRING("title", curNode->name)) {
        piece->fold = 0;
        piece->newline = 0;
      }
      else if (IS_STRING("ul", curNode->name) || IS_STRING("ol", curNode->name)) {  // * unordered list
        piece->fold += 3;
      }
      else if (IS_STRING("p", curNode->name)) {  // * paragraph
      }
      else if (IS_STRING("li", curNode->name)) {  //  * list item
        piece->string = (uint8_t *)"\u00b7";
        piece->newline = 1;
      }
      else if (IS_STRING("code", curNode->name)) {  //  * codeblock
        buffer = xmlBufferCreate();
        xmlNodeDump(buffer, curNode->children->doc, curNode->children, 0, 0);
        piece->firAttr = A_CHARTEXT;
        piece->color = Yellow;
        piece->string = xmlBufferContent(buffer);
        TOG_ATTR(dest->ctnr, piece, TRUE);
        ARRANGE(dest, piece);
        TOG_ATTR(dest->ctnr, piece, FALSE);
        xmlFree(buffer);
        continue;
      }
      else if (IS_STRING("strong", curNode->name)) {  //  * bold
        piece->firAttr = A_BOLD;
      }
      else if (IS_STRING("em", curNode->name)) {  //  * italic
        piece->firAttr = A_ITALIC;
      }
      else if (IS_STRING("u", curNode->name) || IS_STRING("ins", curNode->name)) {  // * underline
        piece->firAttr = A_UNDERLINE;
      }
      else if (IS_STRING("s", curNode->name) || IS_STRING("del", curNode->name)) {  // * strikethrough
        piece->firAttr = A_INVIS;
        piece->secAttr = A_REVERSE;
      }
      else if (IS_STRING("kbd", curNode->name)) {  //  * keyboard input
        piece->firAttr = A_DIM;
      }
      else if (IS_STRING("a", curNode->name)) {  //  * hyperlink
        piece->firAttr = A_UNDERLINE;
        piece->color = Blue;
      }
      else if (IS_STRING("h1", curNode->name)) {  //  * header 1 as .SH "NAME"
        piece->string = (uint8_t *)"NAME";
        piece->fold = 0;
        piece->newline += 1;
        piece->firAttr = A_BOLD;
      }
      else if (IS_STRING("h2", curNode->name)) {  //  * header 2 for other .SH
        piece->fold = 0;
        piece->newline += 1;
        piece->firAttr = A_BOLD;
      }
      else if (IS_STRING("h3", curNode->name)) {  //  * header 3 for .SS
        piece->fold = 3;
        piece->newline += 1;
        piece->firAttr = A_BOLD;
      }
      else if (IS_STRING("h4", curNode->name)) {  //  * header 4 as Sub of .SS
        piece->string = (uint8_t *)"SUB: ";
        piece->length = 6;
        piece->fold = 4;
        piece->newline += 1;
        piece->firAttr = A_BOLD;
      }
      else if (IS_STRING("h5", curNode->name)) {  //  * header 5 as Points in Sub
        piece->string = (uint8_t *)"PT: ";
        piece->length = 5;
        piece->fold = 5;
        piece->newline += 1;
        piece->firAttr = A_BOLD;
      }
      else if (IS_STRING("h6", curNode->name)) {  // * header 6 as Sub of Points
        piece->string = (uint8_t *)"PT SUB: ";
        piece->fold = 6;
        piece->newline += 1;
        piece->firAttr = A_BOLD;
      }
      else {
        if (IS_STRING("img", curNode->name)) {
          ret = GET_PROP(curNode, "alt");
          if (ret = GET_PROP(curNode, "alt"))
            piece->string = ret;
          piece->firAttr = A_UNDERLINE;
          piece->color = Blue;
        }
      }
      tog_attr(dest->ctnr, piece->resetAttr, piece->color, piece->firAttr, piece->secAttr, TRUE);
      if (piece->string != NULL)
        piece->length = strlen((const char *)piece->string);

      ARRANGE(dest, piece);
    }
    else if (curNode->type == XML_TEXT_NODE) {
      if (IS_STRING("li", curNode->parent->name)) {
        // piece->char_prefix = (unsigned char*)"\n\u00b7 ";
      }
      else if (IS_STRING("h1", curNode->parent->name)) {
        piece->newline = 1;
        piece->fold = line_fold;
      }

      if (!IS_STRING("\n", curNode->content)) {
        piece->string = curNode->content;
        if (piece->string != NULL)
          piece->length = strlen((const char *)piece->string);
        ARRANGE(dest, piece);
      }
      // else {
      // piece->string = curNode->content;
      // sdmessage(piece->string);
      // }
    }

    get_content(curNode->children, dest, piece->fold);
    tog_attr(dest->ctnr, piece->resetAttr, piece->color, piece->firAttr, piece->secAttr, FALSE);
    contentsReset(piece);
  }
  contentsFree(piece);
}

int view(const Config *configed, struct buf *ob, int blocks)
{
  int           ymax, xmax;
  int           key;
  int           curLine;
  struct parts *page;
  struct parts *status;
  xmlDoc *      doc;
  xmlNode *     rootNode;

  LIBXML_TEST_VERSION;

  //  Render result
  doc = htmlReadMemory((char *)(ob->data), (int)(ob->size),
                       "input markdown", NULL,
                       HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
  if (doc == NULL) {
    sderror(strerror(errno));
    // sderror("Failed to parse document\n");
    return 1;
  }

  rootNode = xmlDocGetRootElement(doc);
  if (rootNode == NULL) {
    sderror(strerror(errno));
    // sderror("empty document\n");
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
      init_pair(Standard, -1, -1);
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
      init_pair(Standard, -1, -1);
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
  page->ctnr = newpad(page->height, page->width);
  keypad(page->ctnr, TRUE); /* enable arrow keys */

  status = partsNew(1, xmax, ymax, 0); /* status page at bottom */
  status->ctnr = newwin(status->height, status->width, status->curY, status->curX);
  scrollok(status->ctnr, TRUE); /* newline implement as auto refresh */

  refresh();
  wattrset(status->ctnr, A_REVERSE);

  get_content(rootNode, page, configed->line_fold);

  while ((key = wgetch(page->ctnr)) != 'q') {
    switch (key) {
      case 'j':
      case ENTER:
      case KEY_DOWN:
        if (curLine + ymax < page->height)
          curLine++;
        break;
      case KEY_RESIZE:
        halfdelay(3);
        page->resize_key = TRUE;
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
        page->resize_key = FALSE;

        GET_SCREEN_SIZE(ymax, xmax);
        if (status->curY != ymax) {
          mvwin(status->ctnr, ymax, 0);
        }

        if (page->width != xmax) {
          page->width = xmax;
          wresize(page->ctnr, blocks, page->width);
          werase(page->ctnr);
          page->curX = page->curY = 0;
          get_content(rootNode, page, configed->line_fold);
        }
        break;
      default:
        // page->resize_key = FALSE;
        break;
    }
    prefresh(page->ctnr, curLine, 0, 0, 0, ymax, xmax);

    if (ymax >= page->height) {
      wprintw(status->ctnr, "\n Markdown page (ALL) (press q to quit)");
    }
    else if (curLine <= 0) {
      wprintw(status->ctnr, "\n Markdown page (TOP) (press q to quit)");
    }
    else if (curLine + ymax < page->height)
      wprintw(status->ctnr,
              "\n Markdown page (%d%%) (press q to quit)",
              ((curLine + ymax) * 100) / page->height);
    else
      wprintw(status->ctnr, "\n Markdown page (END) (press q to quit)");
    wrefresh(status->ctnr);
  }

  //  Clean up
  xmlFreeDoc(doc);
  partsFree(page);
  partsFree(status);
  endwin();
  return 0;
}

/* vim: set filetype=c: */
