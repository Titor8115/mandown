#include "view.h"

#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "buffer.h"

Part *
part_new(int height, int width, int curY, int curX)
{
  Part *ret;
  ret = malloc(sizeof(Part));

  if (ret) {
    ret->ctnr = NULL;
    ret->height = height;
    ret->width = width;
    ret->curY = curY;
    ret->curX = curX;
  }
  return ret;
}

void part_free(Part *part)
{
  if (!part)
    return;
  delwin(part->ctnr);
  free(part);
}

Content *
content_new(struct buf *buf)
{
  Content *ret;
  ret = malloc(sizeof(Content));

  if (ret) {
    ret->string = buf;
    ret->next = NULL;
    ret->fold = 0;
    ret->color = Standard;
    ret->firAttr = A_NORMAL;
    ret->secAttr = A_NORMAL;
    ret->newline = FALSE;
    ret->togAttr = FALSE;
    ret->formated = FALSE;
  }
  return ret;
}

void content_append(Content **head, Content **content)
{
  Content *p = *head;
  Content *new = *content;
  if (new == NULL)
    return;

  if (*head == NULL) {
    *head = new;
    return;
  }

  while (p->next != NULL)
    p = p->next;
  p->next = new;
}

void content_free(Content *content)
{
  if (!content)
    return;
  bufrelease(content->string);
  content_free(content->next);
  free(content);
}

void content_reset(Content *content)
{
  if (!content)
    return;
  bufrelease(content->string);
  content_free(content->next);
  content->string = NULL;
  content->next = NULL;
  content->fold = 0;
  content->color = Standard;
  content->firAttr = A_NORMAL;
  content->secAttr = A_NORMAL;
  content->newline = FALSE;
  content->togAttr = FALSE;
  content->formated = FALSE;
}

void toggle_attr(WINDOW *dest, bool toggle, int color, int firAttr, int secAttr)
{
  if (toggle) {
    if (color) wattron(dest, COLOR_PAIR(color));
    if (firAttr) wattron(dest, firAttr);
    if (secAttr) wattron(dest, secAttr);
  }
  else {
    if (color) wattroff(dest, COLOR_PAIR(color));
    if (firAttr) wattroff(dest, firAttr);
    if (secAttr) wattroff(dest, secAttr);
  }
}

void move_cursor(Part *part, int i, int j)
{

  if (!part->ctnr || !(i || j))
    return;

  part->curY = i;
  part->curX = j;

  while (wmove(part->ctnr, part->curY, part->curX) == ERR) {
    if (part->curY >= part->height) {
      part->height = part->curY + 1;
    }
    if (part->curX >= part->width - 1) {
      part->curX = 0;
      part->curY++;
    }
    wresize(part->ctnr, part->height, part->width);
  }
}

void render_content(Part *dest, Content *ib)
{
  int      newline;
  int      len;
  size_t   position;
  char *   toc;
  char *   out;
  char *   string = NULL;
  Content *ob;

  dest->curY = dest->curX = 0;
  for (ob = ib; ob != NULL; ob = ob->next) {
    if (ob->newline)
      move_cursor(dest, dest->curY + 1, 0);
    if (!(ob->string)) {
      toggle_attr(dest->ctnr, ob->togAttr, ob->color, ob->firAttr, ob->secAttr);
      continue;
    }
    else if (ob->togAttr)
      toggle_attr(dest->ctnr, TRUE, ob->color, ob->firAttr, ob->secAttr);

    if (ob->string->size) {
      string = calloc(ob->string->size + 1, sizeof(char));
      strncpy(string, (char *)ob->string->data, ob->string->size);
    }
    else
      continue;

    if (ob->formated)
      toc = "\n";
    else
      toc = " ";

    position = 0;
    out = strtok(string, toc);
    while (out != NULL) {
      len = strlen(out);
      newline = len / (dest->width - ob->fold);

      // * do height adjustment, and line-fold prefix
      // getyx(dest->ctnr, dest->curY, dest->curX);
      if (dest->curY + newline >= dest->height) {
        dest->height = dest->curY + newline + 1;
        wresize(dest->ctnr, dest->height, dest->width);
      }

      // * do printing
      if (dest->curX == 0)
        move_cursor(dest, dest->curY, ob->fold);
      else if (dest->curX + len >= dest->width) {
        move_cursor(dest, dest->curY + 1, ob->fold);
      }

      wprintw(dest->ctnr, out);
      // dest->curX += len;
      getyx(dest->ctnr, dest->curY, dest->curX);
      if ((position += len + 1) < ob->string->size) {
        if (toc[0] == '\n')
          move_cursor(dest, dest->curY + 1, ob->fold);
        else {
          dest->curX++;
          waddch(dest->ctnr, toc[0]);
        }
      }
      else
        move_cursor(dest, dest->curY, dest->curX + 1);

      out = strtok(NULL, toc);
    }

    if (string)
      free(string);
  }
  if (dest->height >= dest->curY) {
    dest->height = dest->curY + 1;
    wresize(dest->ctnr, dest->height, dest->width);
  }
}

Content *
get_content(xmlNode *node, int fold)
{
  Content *      head = NULL;
  Content *      piece;
  Content *      tail;
  xmlNode *      curNode;
  xmlBufferPtr   buffer;
  const xmlChar *ret;

  for (curNode = node; curNode != NULL; curNode = curNode->next) {
    tail = content_new(NULL);
    piece = content_new(bufnew(OUTPUT_UNIT));
    piece->fold = fold;

    if (curNode->type == XML_ELEMENT_NODE) {
      piece->togAttr = TRUE;
      if (IS_NODE("body", curNode->parent->name)) {
        piece->newline = tail->newline = TRUE;
      }
      if (IS_NODE("title", curNode->name)) {
        piece->fold = 0;
        tail->newline = TRUE;
      }
      else if (IS_NODE("h1", curNode->name)) {  //  * h1 as "NAME" .SH
        bufputs(piece->string, "\rNAME");
        piece->firAttr = A_BOLD;
        tail->newline = FALSE;
      }
      else if (IS_NODE("h2", curNode->name)) {  //  * h2 for all other .SH
        piece->fold = 0;
        piece->firAttr = A_BOLD;
        tail->newline = FALSE;
      }
      else if (IS_NODE("h3", curNode->name)) {  //  * h3 for .SS
        piece->fold = 3;
        piece->firAttr = A_BOLD;
        tail->newline = FALSE;
      }
      else if (IS_NODE("h4", curNode->name)) {  //  * h4 as Sub of .SS
        bufputs(piece->string, "SC: ");
        piece->fold = 4;
        piece->firAttr = A_BOLD;
        tail->newline = FALSE;
      }
      else if (IS_NODE("h5", curNode->name)) {  //  * h5 as Points in Sub

        bufputs(piece->string, "PT: ");
        piece->fold = 5;
        piece->firAttr = A_BOLD;
        tail->newline = FALSE;
      }
      else if (IS_NODE("h6", curNode->name)) {  // * h6 as Sub of Points
        bufputs(piece->string, "PS: ");
        piece->fold = 6;
        piece->firAttr = A_BOLD;
        tail->newline = FALSE;
      }
      else if (IS_NODE("ul", curNode->name) || IS_NODE("ol", curNode->name)) {  // * unordered list
        piece->fold += 3;
      }
      else if (IS_NODE("li", curNode->name)) {  //  * list item
        bufputs(piece->string, "\u00b7");
        piece->newline = TRUE;
      }
      else if (IS_NODE("p", curNode->name)) {  // * paragraph
        piece->newline = tail->newline = TRUE;
      }
      else if (IS_NODE("strong", curNode->name)) {  //  * bold
        piece->firAttr = A_BOLD;
      }
      else if (IS_NODE("em", curNode->name)) {  //  * italic
        piece->firAttr = A_ITALIC;
      }
      else if (IS_NODE("ins", curNode->name)) {  // * underline (<u> not included)
        piece->firAttr = A_UNDERLINE;
      }
      else if (IS_NODE("del", curNode->name)) {  // * strikethrough (<s> not included)
        piece->firAttr = A_INVIS;
        piece->secAttr = A_REVERSE;
      }
      else if (IS_NODE("kbd", curNode->name)) {  //  * keyboard key
        piece->firAttr = A_DIM;
      }
      else if (IS_NODE("code", curNode->name)) {  //  * codeblock
        buffer = xmlBufferCreate();
        xmlNodeDump(buffer, curNode->doc, curNode->children, 0, 0);
        ret = xmlBufferContent(buffer);
        bufputs(piece->string, (char *)ret);
        piece->color = Yellow;
        piece->firAttr = A_CHARTEXT;
        piece->formated = TRUE;
        COPY_ATTR(tail, piece);
        piece->next = tail;
        content_append(&head, &piece);
        xmlFree(buffer);
        continue;
      }
      else if (IS_NODE("a", curNode->name)) {  //  * hyperlink
        piece->color = Blue;
        piece->firAttr = A_UNDERLINE;
      }
      else if (IS_NODE("img", curNode->name)) {  //  * image
        ret = GET_PROP(curNode, "alt");
        bufput(piece->string, (char *)ret, strlen((char *)ret));
        piece->color = Blue;
        piece->firAttr = A_UNDERLINE;
      }
      if (IS_NODE("a", curNode->parent->name) || IS_NODE("img", curNode->parent->name)) {
        piece->color = Blue;
        piece->firAttr = A_UNDERLINE;
        piece->secAttr = A_NORMAL;
      }
      if (!piece->string->data) {
        bufrelease(piece->string);
        piece->string = NULL;
      }
    }
    else if (curNode->type == XML_TEXT_NODE) {
      ret = curNode->content;
      if (!xmlStrEqual((xmlChar *)"\n", ret))
        bufput(piece->string, (char *)ret, strlen((char *)ret));
      else {
        bufrelease(piece->string);
        piece->string = NULL;
      }

      if (IS_NODE("h1", curNode->parent->name)) {
        piece->newline = TRUE;
      }
    }
    COPY_ATTR(tail, piece);
    piece->next = get_content(curNode->children, piece->fold);
    content_append(&head, &piece);
    content_append(&head, &tail);
  }

  return head;
}

int view(const Config *configed, struct buf *ob, int blocks)
{
  bool     update = TRUE;        /* control needs to update */
  int      ymax, xmax;           /* dimension of stdscr */
  int      key = 0, prevKey = 0; /* input key, previous key */
  int      lineNum = 0;          /* cursor position on page */
  Part *   page;                 /* content container */
  Part *   status;               /* status line container */
  Content *content;
  xmlDoc * doc;
  xmlNode *rootNode;

  LIBXML_TEST_VERSION;

  // * Grow DOM tree;
  doc = htmlReadMemory((char *)(ob->data), (int)(ob->size),
                       "input markdown", NULL,
                       HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
  if (!doc) {
    // sderror(strerror(errno));
    sderror("Failed to parse document");
    return EXIT_FAILURE;
  }

  rootNode = xmlDocGetRootElement(doc);
  if (!rootNode) {
    // sderror(strerror(errno));
    sderror("empty document");
    xmlFreeDoc(doc);
    return EXIT_FAILURE;
  }
  content = get_content(rootNode, configed->lineFold);

  xmlFreeDoc(doc);
  // * Initialize ncurses
  setlocale(LC_ALL, "");
  initscr();
  halfdelay(1);
  noecho();             /* disable echo of keyboard typing */
  keypad(stdscr, TRUE); /* enable arrow keys */
  curs_set(0);          /* disable cursor */

  if (has_colors()) {  // * Set color if terminal support
    start_color();
    use_default_colors();

    if (COLORS == 256) {  //  todo: 256 color mode
      init_pair(Standard, -1, -1);
      init_pair(Red, COLOR_RED, -1);
      init_pair(Green, COLOR_GREEN, -1);
      init_pair(Yellow, COLOR_YELLOW, -1);
      init_pair(Blue, COLOR_BLUE, -1);
      init_pair(Magenta, COLOR_MAGENTA, -1);
      init_pair(Cyan, COLOR_CYAN, -1);
      init_pair(White, COLOR_WHITE, -1);
    }
    else {  // * 8 color mode
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
  page = part_new(blocks, xmax, 0, 0);
  page->ctnr = newpad(page->height, page->width);

  status = part_new(1, xmax, ymax, 0);
  status->ctnr = newwin(status->height, status->width, status->curY, status->curX);
  // scrollok(status->ctnr, TRUE);
  wattrset(status->ctnr, A_REVERSE);

  refresh();

  while ((key = getch()) != 'q') {
    switch (key) {
      case 'j':
      case ENTER:
      case KEY_DOWN: {
        if (lineNum + ymax < page->height)
          lineNum++;
        break;
      }
      case 'k':
      case KEY_BACKSPACE:
      case KEY_UP: {
        if (lineNum > 0)
          lineNum--;
        break;
      }
      case KEY_RESIZE: {
        update = TRUE;
        GET_SCREEN_SIZE(ymax, xmax);
        break;
      }
      case -1: {
        if (update) {
          key = 0;
          update = !update;
          if (prevKey == KEY_RESIZE) {
            // page->height = blocks;
            page->width = xmax;
            wresize(page->ctnr, page->height, page->width);
            wclear(page->ctnr);
            mvwin(status->ctnr, ymax, 0);
          }

          render_content(page, content);
        }
        break;
      }
      default:
        key = -1;
        break;
    }
    if (key != -1) {
      if (ymax >= page->height)
        wprintw(status->ctnr, "\r Markdown page (ALL) (press q to quit)");
      else if (lineNum <= 0)
        wprintw(status->ctnr, "\r Markdown page (TOP) (press q to quit)");
      else if (lineNum + ymax < page->height)
        wprintw(status->ctnr, "\r Markdown page (%d%%) (press q to quit)", ((lineNum + ymax) * 100) / page->height);
      else
        wprintw(status->ctnr, "\r Markdown page (END) (press q to quit)");

      wnoutrefresh(status->ctnr);
      pnoutrefresh(page->ctnr, lineNum, 0, 0, 0, ymax - 1, xmax);
    }
    prevKey = key;
    doupdate();
  }

  //  Clean up
  part_free(page);
  part_free(status);
  endwin();

  content_free(content);

  return EXIT_SUCCESS;
}

/* vim: set filetype=c: */
