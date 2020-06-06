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
content_new()
{
  Content *ret;
  ret = malloc(sizeof(Content));

  if (ret) {
    ret->string = NULL;
    ret->next = NULL;
    ret->fold = 0;
    ret->newline = 0;
    ret->color = Standard;
    ret->firAttr = A_NORMAL;
    ret->secAttr = A_NORMAL;
    ret->togAttr = FALSE;
    ret->formated = FALSE;
  }
  return ret;
}

void content_append(Content **head, Content *content)
{
  Content *p = *head;
  if (*head == NULL) {
    *head = content;
    return;
  }

  while (p->next != NULL)
    p = p->next;
  p->next = content;
}

void content_reset(Content *content)
{
  if (!content)
    return;
  content->string = NULL;
  content->next = NULL;
  content->fold = 0;
  content->newline = 0;
  content->color = Standard;
  content->firAttr = A_NORMAL;
  content->secAttr = A_NORMAL;
  content->togAttr = FALSE;
  content->formated = FALSE;
}

void content_free(Content *content)
{
  if (!content)
    return;
  bufrelease(content->string);
  content_free(content->next);

  free(content);
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

void render_content(Part *dest, Content *ib)
{
  int      i;
  int      len;
  char     toc[3] = {' ', '\n', 0};
  char *   tocptr = toc;
  char *   out;
  char *   string = NULL;
  Content *ob;

  for (ob = ib; ob != NULL; ob = ob->next) {
    for (i = 0; i < ob->newline; i++) {
      wprintw(dest->ctnr, "\n");
    }
    if (!(ob->string)) {
      toggle_attr(dest->ctnr, FALSE, ob->color, ob->firAttr, ob->secAttr);
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

    if (ob->formated) {
      tocptr[0] = '\n';
      tocptr[1] = '\0';
    }
    else {
      tocptr[0] = ' ';
      tocptr[1] = '\n';
    }

    out = strtok(string, toc);
    while (out != NULL) {
      len = strlen(out);

      // * do height adjustment, and line-fold prefix
      getyx(dest->ctnr, dest->curY, dest->curX);
      if (dest->curY > dest->height - 2) {
        dest->height += 1;
        wresize(dest->ctnr, dest->height, dest->width);
      }

      if (dest->curX == 0) {
        dest->curX = ob->fold;
        toggle_attr(dest->ctnr, FALSE, ob->color, ob->firAttr, ob->secAttr);
        wprintw(dest->ctnr, "%*s", ob->fold, "");
        toggle_attr(dest->ctnr, TRUE, ob->color, ob->firAttr, ob->secAttr);
      }

      // * do printing
      if (dest->curX + len > dest->width - 2) {
        dest->height += 2;
        wresize(dest->ctnr, dest->height, dest->width);
        toggle_attr(dest->ctnr, FALSE, ob->color, ob->firAttr, ob->secAttr);
        wprintw(dest->ctnr, "\n%*s", ob->fold, "");
        toggle_attr(dest->ctnr, TRUE, ob->color, ob->firAttr, ob->secAttr);
      }
      wprintw(dest->ctnr, out);
      toggle_attr(dest->ctnr, FALSE, ob->color, ob->firAttr, ob->secAttr);
      waddch(dest->ctnr, tocptr[0]);
      toggle_attr(dest->ctnr, TRUE, ob->color, ob->firAttr, ob->secAttr);
      out = strtok(NULL, toc);
    }
    if (string)
      free(string);
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
    tail = content_new();
    piece = content_new();
    piece->string = bufnew(OUTPUT_UNIT);
    piece->fold = fold;

    if (curNode->type == XML_ELEMENT_NODE) {
      piece->togAttr = TRUE;

      if (IS_NODE("article", curNode->parent->name)) {
        piece->fold = fold;
        piece->newline = 1;
      }
      if (IS_NODE("title", curNode->name)) {
        piece->fold = 0;
        piece->newline = 0;
      }
      else if (IS_NODE("ul", curNode->name) || IS_NODE("ol", curNode->name)) {  // * unordered list
        piece->fold += 3;
      }
      else if (IS_NODE("p", curNode->name)) {  // * paragraph
      }
      else if (IS_NODE("li", curNode->name)) {  //  * list item
        bufputs(piece->string, "\u00b7");
        piece->newline = 1;
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
        content_append(&head, piece);
        xmlFree(buffer);
        continue;
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
      else if (IS_NODE("a", curNode->name)) {  //  * hyperlink
        piece->color = Blue;
        piece->firAttr = A_UNDERLINE;
      }
      else if (IS_NODE("h1", curNode->name)) {  //  * h1 as "NAME" .SH
        bufputs(piece->string, "\rNAME\n");
        piece->newline += 1;
        piece->firAttr = A_BOLD;
      }
      else if (IS_NODE("h2", curNode->name)) {  //  * h2 for all other .SH
        piece->fold = 0;
        piece->newline += 1;
        piece->firAttr = A_BOLD;
      }
      else if (IS_NODE("h3", curNode->name)) {  //  * h3 for .SS
        piece->fold = 3;
        piece->newline += 1;
        piece->firAttr = A_BOLD;
      }
      else if (IS_NODE("h4", curNode->name)) {  //  * h4 as Sub of .SS
        bufputs(piece->string, "SUB: ");
        piece->fold = 4;
        piece->newline += 1;
        piece->firAttr = A_BOLD;
      }
      else if (IS_NODE("h5", curNode->name)) {  //  * h5 as Points in Sub

        bufputs(piece->string, "PT: ");
        piece->fold = 5;
        piece->newline += 1;
        piece->firAttr = A_BOLD;
      }
      else if (IS_NODE("h6", curNode->name)) {  // * h6 as Sub of Points
        bufputs(piece->string, "PT SUB: ");
        piece->fold = 6;
        piece->newline += 1;
        piece->firAttr = A_BOLD;
      }
      else if (IS_NODE("img", curNode->name)) {
        ret = GET_PROP(curNode, "alt");
        bufput(piece->string, (char *)ret, strlen((char *)ret));
        piece->color = Blue;
        piece->firAttr = A_UNDERLINE;
      }
    }
    else if (curNode->type == XML_TEXT_NODE) {
      if (IS_NODE("h1", curNode->parent->name))
      {
        piece->newline = 1;
        piece->firAttr = A_BOLD;
        piece->fold = fold;
      }

      ret = curNode->content;
      if (!IS_NODE("\n", ret)) {
        bufput(piece->string, (char *)ret, strlen((char *)ret));
      }
    }
    COPY_ATTR(tail, piece);
    content_append(&head, piece);
    content_append(&head, get_content(curNode->children, piece->fold));

    content_append(&head, tail);
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
  scrollok(status->ctnr, TRUE);
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
            page->height = blocks;
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
        wprintw(status->ctnr, "\n Markdown page (ALL) (press q to quit)");
      else if (lineNum <= 0)
        wprintw(status->ctnr, "\n Markdown page (TOP) (press q to quit)");
      else if (lineNum + ymax < page->height)
        wprintw(status->ctnr, "\n Markdown page (%d%%) (press q to quit)", ((lineNum + ymax) * 100) / page->height);
      else
        wprintw(status->ctnr, "\n Markdown page (END) (press q to quit)");

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
