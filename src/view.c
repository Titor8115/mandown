#include "view.h"

#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <panel.h>

#include "buffer.h"
#include "stack.h"
#include "dom.h"



enum
{
  C_DEFAULT = 0,
  C_RED,
  C_GREEN,
  C_YELLOW,
  C_BLUE,
  C_MAGENTA,
  C_CYAN,

  C_MAX,
};

/* Available colors: BLACK RED GREEN YELLOW BLUE MAGENTA CYAN WHITE */
static const struct { short fg; short bg; } color_pairs[] =
{
  /* color */     /* foreground*/ /* background*/
  [C_DEFAULT]   = { -1,                     -1 },
  [C_RED]       = { COLOR_RED,              -1 },
  [C_GREEN]     = { COLOR_GREEN,            -1 },
  [C_YELLOW]    = { COLOR_YELLOW,           -1 },
  [C_BLUE]      = { COLOR_BLUE,             -1 },
  [C_MAGENTA]   = { COLOR_MAGENTA,          -1 },
  [C_CYAN]      = { COLOR_CYAN,             -1 },
};

enum
{
  FRAME_NONE,
  FRAME_HELP,
  FRAME_STAT,
  FRAME_PAGE,
  FRAME_HREF,
};

static const char frame_title[][19] =
{
  [FRAME_NONE]    = "",
  [FRAME_HELP]    = " Help ",
  [FRAME_PAGE]    = " struct dom_link_t ",
  [FRAME_STAT]    = " Markdown page ",
  [FRAME_HREF]    = " Reference ",
};

static const attr_t frame_attr[] =
{
  [FRAME_NONE]    = A_NORMAL,
  [FRAME_HELP]    = A_NORMAL,
  [FRAME_PAGE]    = A_NORMAL,
  [FRAME_STAT]    = A_REVERSE,
  [FRAME_HREF]    = A_REVERSE,
};

enum {
  N_PLAIN = 0,
  N_EM,
  N_STRONG,
  N_INS,
  N_DEL,
  N_CODE,
  N_KBD,

  N_HEADING,
  N_LINK
};

static const attr_t node_attr[] =
{
  [N_EM]        = A_ITALIC,
  [N_STRONG]    = A_BOLD,
  [N_INS]       = A_UNDERLINE,
  [N_DEL]       = A_INVIS     | A_REVERSE,
  [N_CODE]      = A_NORMAL    | COLOR_PAIR(C_YELLOW),
  [N_KBD]       = A_DIM,

  [N_HEADING]   = A_BOLD,
  [N_LINK]      = A_UNDERLINE | COLOR_PAIR(C_BLUE),
};


struct frame_t {
  WINDOW *win;
  attr_t  attr;
  int     frame_type;
  int     height;
  int     width;
  int     beg_y;
  int     beg_x;
  int     cur_y;
  int     cur_x;
};

static void
create_color_pairs()
{
  if (has_colors()) {  // * Set color if terminal support
    start_color();
    use_default_colors();
    for (int i = C_DEFAULT; i < C_MAX; i++)
      init_pair(i, color_pairs[i].fg, color_pairs[i].bg);
  }
  else {
    sdwarn("Terminal doesn't seem to support colors");
    return;
  }
}

struct frame_t *
frame_new(int type, int height, int width, int begin_y, int begin_x)
{
  struct frame_t *ret;
  ret = malloc(sizeof(struct frame_t));

  if (ret) {
    ret->win      = NULL;
    ret->attr       = frame_attr[type];
    ret->frame_type = type;
    ret->height     = height;
    ret->width      = width;
    ret->beg_y = ret->cur_y = begin_y;
    ret->beg_x = ret->cur_x = begin_x;
  }
  return ret;
}

void
frame_free(struct frame_t *part)
{
  if (!part)
    return;
  delwin(part->win);
  free(part);
}

static inline void
toggle_attr(WINDOW *dest, int toggle, int attributes)
{
  (toggle) ? wattron(dest, attributes) : wattroff(dest, attributes);
}

static void
move_cursor(struct frame_t *part, int cur_y, int cur_x)
{
  if (!part->win)
    return;

  while (wmove(part->win, cur_y, cur_x) == ERR) {
    if (cur_x >= part->width) {
      cur_x = 0;
      cur_y++;
    }

    if (cur_y >= part->height) {
      part->height = cur_y + 2;
    }
    wresize(part->win, part->height, part->width);
  }
}

static void
render_content(struct frame_t *dest, struct dom_link_t *ib)
{
  int      cur_y = 0;
  int      cur_x = 0;
  int      len;
  size_t   position;
  char *   toc = " \n";
  char *   out;
  char *   string = NULL;
  struct dom_link_t *ob;

  for (ob = ib; ob != NULL; ob = ob->next) {
    if (ob->prop & P_CONTROL) {
      toggle_attr(dest->win, (ob->prop & P_ATTR_ON), ob->buf_attr);
      toc = (ob->prop & P_SELF_FORMAT) ? "\n" : " \n";
    }

    if (ob->prop & P_LINE_BREAK) {
      // cur_x = ob->fold;
      move_cursor(dest, ++cur_y, cur_x = 0);
    }


    if (!ob->buf)
      continue;
    else {
      string = calloc(ob->buf->size + 1, sizeof(char));
      strncpy(string, (char *)ob->buf->data, ob->buf->size);
    }


    position = 0;
    out = strtok(string, toc);
    while (out != NULL) {
      len = strlen(out);

      if (cur_x == 0) move_cursor(dest, cur_y, cur_x = ob->fold);

      if ((cur_x > ob->fold) && (cur_x + len >= dest->width)) {
        cur_x = ob->fold;
        cur_y ++;
        move_cursor(dest, cur_y + (len + ob->fold) / dest->width, cur_x);
      }

      waddnstr(dest->win, out, len);
      // j += len;
      getyx(dest->win, cur_y, cur_x);
      position += len;
      if (position < ob->buf->size) {
        if (toc[0] == '\n') {
          // cur_x = ob->fold;
          move_cursor(dest, ++cur_y, cur_x = 0);
        }
        else if (cur_x < dest->width) {
          cur_x++;
          waddch(dest->win, toc[0]);
        }
      }
      else
        move_cursor(dest, cur_y, ++cur_x);
      
      position++;

      out = strtok(NULL, toc);
    }

    if (string)
      free(string);
  }
  if (dest->height >= cur_y) {
    dest->height = cur_y + 1;
    wresize(dest->win, dest->height, dest->width);
  }
}

struct dom_link_t *
get_content(xmlNode *node, int fold)
{
  size_t             len;
  struct dom_link_t *link;
  struct dom_link_t *head     = NULL;
  struct dom_link_t *tail     = NULL;
  htmlNodePtr        cur_node = node;
  const xmlChar *    ret;

  while (cur_node != NULL) {
    link = dom_link_new(bufnew(OUTPUT_UNIT));
    link->fold = fold;

    if (cur_node->type == XML_ELEMENT_NODE) {
      link->prop = (P_CONTROL | P_ATTR_ON);
      tail = dom_link_new(NULL);
      tail->prop = P_CONTROL;

      if (cmp_xml("body", cur_node->parent->name)) {
        link->prop |= P_LINE_BREAK;
        tail->prop |= P_LINE_BREAK;
      }
      // if (cmp_xml("html", cur_node->name)) {
      // }
      // else if (cmp_xml("head", cur_node->name)) {
      // }
      // else if (cmp_xml("body", cur_node->name)) {
      // }

      if (cmp_xml("title", cur_node->name)) {
        link->fold = 0;
        tail->prop ^= P_LINE_BREAK;
      }
      else if (cmp_xml("h1", cur_node->name)) {  //  * h1 as "NAME" .SH
        bufputs(link->buf, "\rNAME");
        link->buf_attr = node_attr[N_HEADING];
        tail->prop ^= P_LINE_BREAK;
      }
      else if (cmp_xml("h2", cur_node->name)) {  //  * h2 for all other .SH
        link->fold = 0;
        link->buf_attr = node_attr[N_HEADING];
        tail->prop ^= P_LINE_BREAK;
      }
      else if (cmp_xml("h3", cur_node->name)) {  //  * h3 for .SS
        link->fold = 3;
        link->buf_attr = node_attr[N_HEADING];
        tail->prop ^= P_LINE_BREAK;
      }
      else if (cmp_xml("h4", cur_node->name)) {  //  * h4 as Sub of .SS
        bufputs(link->buf, "SC: ");
        link->fold = 4;
        link->buf_attr = node_attr[N_HEADING];
        tail->prop ^= P_LINE_BREAK;
      }
      else if (cmp_xml("h5", cur_node->name)) {  //  * h5 as Points in Sub

        bufputs(link->buf, "PT: ");
        link->fold = 5;
        link->buf_attr = node_attr[N_HEADING];
        tail->prop ^= P_LINE_BREAK;
      }
      else if (cmp_xml("h6", cur_node->name)) {  // * h6 as Sub of Points
        bufputs(link->buf, "PS: ");
        link->fold = 6;
        link->buf_attr = node_attr[N_HEADING];
        tail->prop ^= P_LINE_BREAK;
      }
      else if (cmp_xml("ul", cur_node->name) || cmp_xml("ol", cur_node->name)) {  // * unordered list
        link->fold += 3;
      }
      else if (cmp_xml("li", cur_node->name)) {  //  * list item
        bufputs(link->buf, "\u00b7");
        link->prop |= P_LINE_BREAK;
      }
      else if (cmp_xml("p", cur_node->name)) {  // * paragraph
        link->buf_attr = node_attr[N_PLAIN];
      }
      else if (cmp_xml("strong", cur_node->name) || cmp_xml("b", cur_node->name)) {  //  * bold
        link->buf_attr = node_attr[N_STRONG];
      }
      else if (cmp_xml("em", cur_node->name) || cmp_xml("i", cur_node->name)) {  //  * italic
        link->buf_attr = node_attr[N_EM];
      }
      else if (cmp_xml("ins", cur_node->name) || cmp_xml("u", cur_node->name)) {  // * underline (<u> not included)
        link->buf_attr = node_attr[N_INS];
      }
      else if (cmp_xml("del", cur_node->name) || cmp_xml("s", cur_node->name)) {  // * strikethrough (<s> not included)
        link->buf_attr = node_attr[N_DEL];
      }
      else if (cmp_xml("kbd", cur_node->name)) {  //  * keyboard key
        link->buf_attr = node_attr[N_KBD];
      }
      else if (cmp_xml("pre", cur_node->name)) {  //  * codeblock
        link->prop |= P_SELF_FORMAT | P_LINE_BREAK;
        link->fold += 3;
      }
      else if (cmp_xml("code", cur_node->name)) {  //  * codeblock
        link->buf_attr = node_attr[N_CODE];
        link->prop |= P_SELF_FORMAT;
      }
      else if (cmp_xml("a", cur_node->name)) {  //  * hyperlink
        link->buf_attr = node_attr[N_LINK];
      }
      else if (cmp_xml("img", cur_node->name)) {  //  * image
        ret = get_prop(cur_node, "alt");
        len = strlen((char *)ret);
        bufput(link->buf, (char *)ret, len);
        link->buf_attr = node_attr[N_LINK];
      }

      if (cmp_xml("a", cur_node->parent->name) || cmp_xml("img", cur_node->parent->name)) {
        link->buf_attr = node_attr[N_LINK];
      }
      if (!link->buf->size) {
        bufrelease(link->buf);
        link->buf = NULL;
      }
    }
    else if (cur_node->type == XML_TEXT_NODE) {
      ret = cur_node->content;
      bufput(link->buf, (char *)ret, strlen((char *)ret));

      if (cmp_xml("h1", cur_node->parent->name)) {
        link->prop = P_LINE_BREAK;
      }
      else if (cmp_xml("code", cur_node->parent->name)) {
                link->prop |= P_SELF_FORMAT;
      }
    }
    link->next = get_content(cur_node->children, link->fold);
    dom_link_append(&head, link);
    if (tail) {
      tail->buf_attr  = link->buf_attr;
      dom_link_append(&head, tail);
      tail = tail->next;
    }
    cur_node = cur_node->next;
  }

  return head;
}

int view(const Config *configed, struct buf *ob, int href_count)
{
  bool        update = TRUE; /* control needs to update */
  int         height, width; /* dimension of stdscr */
  int         key;           /* input key*/
  int         prevKey = OK;  /* previous key */
  struct frame_t *      page;          /* content container */
  struct frame_t *      status;        /* status line container */
  struct dom_link_t *   content;
  htmlDocPtr  doc;
  htmlNodePtr rootNode;

  LIBXML_TEST_VERSION;

  // * Grow DOM tree;
  doc = htmlReadMemory((char *)(ob->data), (int)(ob->size),
                       "input markdown", NULL, HTML_PARSE_NOBLANKS);
  if (!doc) {
    // sderror(strerror(errno));
    sderror("Failed to parse document");
    return EXIT_FAILURE;
  }

  rootNode = xmlDocGetRootElement(doc);
  if (!rootNode) {
    sderror("empty document");
    xmlFreeDoc(doc);
    return EXIT_FAILURE;
  }
  bufreset(ob);

  // if (href_count)
  //   stack_init(&url, href_count);
  content = get_content(rootNode, configed->lineFold);
  xmlFreeDoc(doc);

  setlocale(LC_ALL, "");
  initscr();            /* Initialize ncurses */
  noecho();             /* disable echo of keyboard typing */
  keypad(stdscr, TRUE); /* enable arrow keys */
  timeout(200);
  cbreak();
  // halfdelay(1);
  curs_set(0);

  create_color_pairs();

  get_stdscr_size(height, width);

  page        = frame_new(FRAME_NONE, 1, width, 0, 0);
  page->win   = newpad(page->height, page->width);
  status      = frame_new(FRAME_STAT, 1, width, height, 0);
  status->win = newwin(status->height, status->width, status->cur_y, status->cur_x);
  scrollok(status->win, TRUE);
  wattrset(status->win, status->attr);

  refresh();

  while ((key = getch()) != 'q') {
    switch (key) {
      case 'j':
      case ENTER:
      case KEY_DOWN: {
        if (page->cur_y + height < page->height)
          page->cur_y++;
        break;
      }

      case 'k':
      // case KEY_BACKSPACE:
      case KEY_UP: {
        if (page->cur_y > 0)
          page->cur_y--;
        break;
      }

      case ' ':
      case KEY_NPAGE: {
        if (page->cur_y + ((height - 1) << 1) < page->height)
          page->cur_y += (height - 1);
        else
          page->cur_y = page->height - height;
        break;
      }

      case KEY_BACKSPACE:
      case KEY_PPAGE: {
        if (page->cur_y - (height - 1) > 0)
          page->cur_y -= (height - 1);
        else
          page->cur_y = 0;
        break;
      }

      case KEY_RESIZE: {
        update = TRUE;
        get_stdscr_size(height, width);
        page->width = status->width = width;
        break;
      }
      case ERR: {
        if (update) {
          key    = OK;
          update = !update;
          if (prevKey == KEY_RESIZE) {
            // page->height = blocks;
            wresize(page->win, page->height, page->width);
            wresize(status->win, status->height, status->width);
            wclear(page->win);
            mvwin(status->win, height, 0);
          }

          render_content(page, content);
        }
        break;
      }
      default:
        key = ERR;
        break;
    }
    if (key != ERR) {
      if (height >= page->height)
        wprintw(status->win, "\n%s(ALL) (q to quit)", frame_title[status->frame_type]);
      else if (page->cur_y <= 0)
        wprintw(status->win, "\n%s(TOP) (q to quit)", frame_title[status->frame_type]);
      else if (page->cur_y + height < page->height)
        wprintw(status->win, "\n%s(%d%%) (q to quit)", frame_title[status->frame_type], ((page->cur_y + height) * 100) / page->height);
      else
        wprintw(status->win, "\n%s(END) (q to quit)", frame_title[status->frame_type]);

      pnoutrefresh(page->win, page->cur_y, 0, 0, 0, height - 1, width);
      wnoutrefresh(status->win);
    }
    prevKey = key;
    doupdate();
  }

  //  Clean up
  endwin();
  frame_free(page);
  frame_free(status);

  dom_link_free(content);

  return EXIT_SUCCESS;
}

/* vim: set filetype=c: */
