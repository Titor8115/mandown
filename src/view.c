#include "view.h"

#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "dom.h"
#include "stack.h"

/* Available colors: BLACK RED GREEN YELLOW BLUE MAGENTA CYAN WHITE */
static const struct {short fg; short bg;} color_pairs[] =
{
  /* color */   /* foreground*/ /* background*/
  [C_DEFAULT] = {-1,                        -1},
  [C_RED]     = {COLOR_RED,                 -1},
  [C_GREEN]   = {COLOR_GREEN,               -1},
  [C_YELLOW]  = {COLOR_YELLOW,              -1},
  [C_BLUE]    = {COLOR_BLUE,                -1},
  [C_MAGENTA] = {COLOR_MAGENTA,             -1},
  [C_CYAN]    = {COLOR_CYAN,                -1},
};

static const char frame_title[][19] =
{
  [FRAME_NONE]  = "",
  [FRAME_HELP]  = " Help ",
  [FRAME_PAGE]  = " struct dom_link ",
  [FRAME_STATS] = " Markdown page ",
  [FRAME_PROBE] = " Reference ",
};

static const attr_t frame_attr[] =
{
  [FRAME_NONE]    = A_NORMAL,
  [FRAME_HELP]    = A_NORMAL,
  [FRAME_PAGE]    = A_NORMAL,
  [FRAME_STATS]    = A_REVERSE,
  [FRAME_PROBE]    = A_REVERSE,
};

static const struct {short pr; attr_t at;} node_attr[] =
{
  [N_PLAIN]   = {P_REGULAR | P_SPLIT,                             A_NORMAL},
  [N_EM]      = {P_CONTROL,                                       A_ITALIC},
  [N_BOLD]    = {P_CONTROL,                                         A_BOLD},
  [N_INS]     = {P_CONTROL,                                    A_UNDERLINE},
  [N_DEL]     = {P_CONTROL,                            A_INVIS | A_REVERSE},
  [N_PRE]     = {P_CONTROL | P_SELF_FORM | P_SPLIT,               A_NORMAL},
  [N_KBD]     = {P_CONTROL,                                          A_DIM},
  [N_HEADING] = {P_CONTROL,                                         A_BOLD},
  [N_HREF]    = {P_CONTROL | P_HYPERLINK, A_UNDERLINE | COLOR_PAIR(C_BLUE)},
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

struct frame *
frame_new(int type, int height, int width, int begin_y, int begin_x)
{
  struct frame *ret;
  ret = malloc(sizeof(struct frame));

  if (ret) {
    ret->win        = NULL;
    ret->attr       = frame_attr[type];
    ret->frame_type = type;
    ret->height     = height;
    ret->width      = width;
    ret->beg_y = ret->cur_y = begin_y;
    ret->beg_x = ret->cur_x = begin_x;
  }
  return ret;
}

void frame_free(struct frame *part)
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
move_cursor(struct frame *part, int cur_y, int cur_x)
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
render_content(struct frame *dest, struct dom_link *ib, struct stack *href_table)
{
  int                    cur_y = 0;
  int                    cur_x = 0;
  int                    len;
  size_t                 stack_pos = 0;
  size_t                 position;
  char *                 toc = " \n";
  char *                 out;
  char *                 tmp = malloc(sizeof(char) * OUTPUT_UNIT);
  struct dom_link *      ob;
  struct dom_href_stack *href = NULL;

  for (ob = ib; ob != NULL; ob = ob->next) {
    if (ob->prop & P_CONTROL) {
      toggle_attr(dest->win, (ob->prop & P_BEG_A), ob->attr);
      toc = (ob->prop & P_SELF_FORM) ? "\n" : " \n";

      if ((ob->prop & P_HYPERLINK) && (ob->prop &  P_BEG_A)) {
        href        = href_table->item[stack_pos];
        stack_pos++;
      }
    }

    if (ob->prop & P_SPLIT) {
      // cur_x = ob->fold;
      move_cursor(dest, ++cur_y, cur_x = ob->fold);
    }

    if (!ob->buf)
      continue;
    else {
      tmp = realloc(tmp, ob->buf->size + 1);
      // tmp = calloc(ob->buf->size + 1, sizeof(char));
      memcpy(tmp, ob->buf->data, ob->buf->size + 1);
    }

    position = 0;
    out      = strtok(tmp, toc);
    while (out != NULL) {
      len = strlen(out);

      if (cur_x == 0) move_cursor(dest, cur_y, cur_x = ob->fold);

      if ((cur_x > ob->fold) && (cur_x + len >= dest->width)) {
        cur_x = ob->fold;
        cur_y++;
        move_cursor(dest, cur_y + (len + ob->fold) / dest->width, cur_x);
      }
        if (href != NULL) {
        href->beg_y = cur_y;
        href->beg_x = cur_x;
        // href->index = stack_pos;
        href = NULL;
        }

      waddnstr(dest->win, out, len);
      getyx(dest->win, cur_y, cur_x);
      // j += len;
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
  }
  free(tmp);

  if (dest->height != cur_y) {
    dest->height = cur_y + 1;
    wresize(dest->win, dest->height, dest->width);
  }
}

struct dom_link *
get_content(xmlNode *node, int fold, struct stack *href_table)
{
  struct dom_href_stack *href;
  struct dom_link *      link;
  struct dom_link *      head = NULL;
  struct dom_link *      tail = NULL;
  htmlNodePtr            cur_node;
  xmlChar *              xml_prop;

  for (cur_node = node; cur_node != NULL; cur_node = cur_node->next) {
    link       = dom_link_new(bufnew(OUTPUT_UNIT));
    link->fold = fold;
    if (cur_node->type == XML_ELEMENT_NODE) {
      link->prop = P_BEG_A;
      tail       = dom_link_new(NULL);
      tail->prop = P_CONTROL;

      if (cmp_xml("body", cur_node->parent->name)) {
        link->prop |= P_SPLIT;
        tail->prop |= P_SPLIT;
      }
      // if (cmp_xml("html", cur_node->name)) {
      // }
      // else if (cmp_xml("head", cur_node->name)) {
      // }
      // else if (cmp_xml("body", cur_node->name)) {
      // }

      if (cmp_xml("title", cur_node->name)) {
        link->fold = 0;
        tail->prop |= P_SPLIT;
      }
      else if (cmp_xml("h1", cur_node->name)) {  //  * h1 as "NAME" .SH
        bufputs(link->buf, "\rNAME");
        link->attr |= node_attr[N_HEADING].at;
        link->prop |= node_attr[N_HEADING].pr;
        tail->prop ^= P_SPLIT;
      }
      else if (cmp_xml("h2", cur_node->name)) {  //  * h2 for all other .SH
        link->fold = 0;
        link->attr |= node_attr[N_HEADING].at;
        link->prop |= node_attr[N_HEADING].pr;
        tail->prop ^= P_SPLIT;
      }
      else if (cmp_xml("h3", cur_node->name)) {  //  * h3 for .SS
        link->fold = 3;
        link->attr |= node_attr[N_HEADING].at;
        link->prop |= node_attr[N_HEADING].pr;
        tail->prop ^= P_SPLIT;
      }
      else if (cmp_xml("h4", cur_node->name)) {  //  * h4 as Sub of .SS
        bufputs(link->buf, "SC: ");
        link->fold = 4;
        link->attr |= node_attr[N_HEADING].at;
        link->prop |= node_attr[N_HEADING].pr;
        tail->prop ^= P_SPLIT;
      }
      else if (cmp_xml("h5", cur_node->name)) {  //  * h5 as Points in Sub

        bufputs(link->buf, "PT: ");
        link->fold = 5;
        link->attr |= node_attr[N_HEADING].at;
        link->prop |= node_attr[N_HEADING].pr;
        tail->prop ^= P_SPLIT;
      }
      else if (cmp_xml("h6", cur_node->name)) {  // * h6 as Sub of Points
        bufputs(link->buf, "PS: ");
        link->fold = 6;
        link->attr |= node_attr[N_HEADING].at;
        link->prop |= node_attr[N_HEADING].pr;
        tail->prop ^= P_SPLIT;
      }
      else if (cmp_xml("ul", cur_node->name) || cmp_xml("ol", cur_node->name)) {  // * unordered list
        link->fold += 2;
      }
      else if (cmp_xml("li", cur_node->name)) {  //  * list item
        bufputs(link->buf, "\u00b7");
        link->prop |= P_SPLIT;
      }
      else if (cmp_xml("p", cur_node->name)) {  // * paragraph
        link->attr |= node_attr[N_PLAIN].at;
        // link->prop |= node_attr[N_PLAIN].pr;
      }
      else if (cmp_xml("strong", cur_node->name) || cmp_xml("b", cur_node->name)) {  //  * bold
        link->attr |= node_attr[N_BOLD].at;
        link->prop |= node_attr[N_BOLD].pr;
      }
      else if (cmp_xml("em", cur_node->name) ||
               cmp_xml("i", cur_node->name)) {  //  * italic
        link->attr |= node_attr[N_EM].at;
        link->prop |= node_attr[N_EM].pr;
      }
      else if (cmp_xml("ins", cur_node->name) ||
               cmp_xml("u", cur_node->name)) {  // * underline
        link->attr |= node_attr[N_INS].at;
        link->prop |= node_attr[N_INS].pr;
      }
      else if (cmp_xml("del", cur_node->name) ||
               cmp_xml("s", cur_node->name)) {  // * strikethrough
        link->attr |= node_attr[N_DEL].at;
        link->prop |= node_attr[N_DEL].pr;
      }
      else if (cmp_xml("kbd", cur_node->name)) {  //  * keyboard key
        link->attr |= node_attr[N_KBD].at;
        link->prop |= node_attr[N_KBD].pr;
      }
      else if (cmp_xml("pre", cur_node->name)) {  //  * codeblock
        link->prop |= P_SELF_FORM | P_SPLIT;
        link->fold += 2;
      }
      else if (cmp_xml("code", cur_node->name)) {  //  * codeblock
        link->attr |= node_attr[N_PRE].at | COLOR_PAIR(C_YELLOW);
        link->prop |= node_attr[N_PRE].pr ^ P_SPLIT;
      }
      else if (cmp_xml("a", cur_node->name)) {  //  * hyperlink
        link->attr |= node_attr[N_HREF].at;
        link->prop |= node_attr[N_HREF].pr;

        xml_prop = get_prop(cur_node, "href");
        if (xml_prop != NULL) {
          href = dom_href_new(bufnew(OUTPUT_UNIT));
          href->index = href_table->size - 1;
          bufputs(href->url, (char *)xml_prop);
          stack_push(href_table, href);
        }
        xmlFree(xml_prop);
      }
      else if (cmp_xml("img", cur_node->name)) {  //  * image
        link->attr |= node_attr[N_HREF].at;
        link->prop |= node_attr[N_HREF].pr;
        xml_prop = get_prop(cur_node, "alt");
        if (xml_prop != NULL) {
          bufputs(link->buf, (char *)xml_prop);
        }
        xml_prop = get_prop(cur_node, "src");
        if (xml_prop != NULL) {
          href = dom_href_new(bufnew(OUTPUT_UNIT));
          href->index = href_table->size - 1;
          bufputs(href->url, (char *)xml_prop);
          stack_push(href_table, href);
        }
        xmlFree(xml_prop);
      }

      if (cmp_xml("a", cur_node->parent->name) ||
          cmp_xml("img", cur_node->parent->name)) {  //  * style overwrite if is href content
        link->attr |= node_attr[N_HREF].at;
      }
    }
    else if (cur_node->type == XML_TEXT_NODE) {
      link->prop = node_attr[N_PLAIN].at;
      bufputs(link->buf, (char *)cur_node->content);

      if (cmp_xml("h1", cur_node->parent->name)) {
        link->prop |= P_SPLIT;
        link->fold = fold;
      }
    }

    if (link->buf->size == 0) {
      bufrelease(link->buf);
      link->buf = NULL;
    }
    else
      bufcstr(link->buf);
    link->next = get_content(cur_node->children, link->fold, href_table);
    dom_link_append(&head, link);

    if (tail) {
      tail->attr = link->attr;
      dom_link_append(&head, tail);
      tail = tail->next;
    }
  }

  return head;
}

int view(const struct config *config, const struct buf *ob, int href_count)
{
  int                    ret    = 0;
  bool                   update = TRUE; /* control needs to update */
  int                    height, width; /* dimension of stdscr */
  int                    key;           /* input key*/
  int                    prevKey = OK;  /* previous key */
  int                    stack_index = 0;
  struct frame_main      mdn     = {NULL, NULL, 0, 0, 0, 0};
  struct frame *         page;   /* content container */
  struct frame *         status; /* status line container */
  struct frame *         probe = NULL;
  struct dom_link *      content;
  htmlDocPtr             doc;
  htmlNodePtr            rootNode;
  struct stack           ref_stack;
  struct dom_href_stack *test;
  // MEVENT            event;

  LIBXML_TEST_VERSION;

  // * Grow DOM tree;
  doc = htmlReadMemory((char *)(ob->data), (int)(ob->size),
                       "file", NULL, HTML_PARSE_NOBLANKS | HTML_PARSE_NOWARNING | HTML_PARSE_RECOVER);
  if (!doc) {
    sderror(&ret, "Failed to make sense of given file as HTML");
    return ret;
  }

  rootNode = xmlDocGetRootElement(doc);
  if (!rootNode) {
    xmlFreeDoc(doc);
    sderror(&ret, "empty document");
    return ret;
  }

  stack_init(&ref_stack, href_count);
  content = get_content(rootNode, config->fold, &ref_stack);

  setlocale(LC_ALL, "");
  initscr();            /* Initialize ncurses */
  noecho();             /* disable echo of keyboard typing */
  keypad(stdscr, TRUE); /* enable arrow keys */
  timeout(200);
  cbreak();
  // halfdelay(1);
  // curs_set(0);

  create_color_pairs();

  get_display_size(height, width);

  page        = frame_new(FRAME_NONE, (int)ob->size / width, width, 0, 0);
  page->win   = newpad(page->height, page->width);

  status      = frame_new(FRAME_STATS, 1, width, height, 0);
  status->win = newwin(status->height, status->width, status->cur_y, status->cur_x);
  scrollok(status->win, TRUE);
  wattrset(status->win, status->attr);

  probe        = frame_new(FRAME_NONE, 4, width, height - 4, 0);
  probe->win   = newwin(probe->height, probe->width, probe->beg_y, probe->beg_x);
  probe->cur_y = probe->cur_x = 1;

  mdn.pad = page;
  mdn.bar = status;
  render_content(page, content, &ref_stack);

  refresh();
  // mousemask(ALL_MOUSE_EVENTS, NULL);

  while ((key = getch()) != 'q') {
    switch (key) {
      // case KEY_MOUSE:
      //   if (getmouse(&event) == OK) {
      //     if (event.bstate & BUTTON1_PRESSED)
      //   }
      case TAB: {
          test = dom_stack_find(&ref_stack, stack_index, page->cur_y, page->cur_y + height, page->cur_x);
          stack_index = test->index + 1;
          if (test != NULL) {
            mdn.cur_y = test->beg_y - page->cur_y;
            mdn.cur_x = test->beg_x - page->cur_x;
            move(mdn.cur_y, mdn.cur_x);
          }
        break;
      }
      case ENTER: {
        if (test != NULL) {
          mvwprintw(probe->win, probe->cur_y, probe->cur_x, (char *)test->url->data);
          wclrtobot(probe->win);
        }
        box(probe->win, 0, 0);
        mvwprintw(probe->win, probe->cur_y - 1, 2, frame_title[FRAME_PROBE]);

        break;
      }
      case 'j':
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
      case 'f':
      case KEY_NPAGE: {
        if (page->cur_y + ((height - 1) << 1) < page->height)
          page->cur_y += (height - 1);
        else
          page->cur_y = page->height - height;
        break;
      }

      case KEY_BACKSPACE:
      case 'b':
      case KEY_PPAGE: {
        if (page->cur_y - (height - 1) > 0)
          page->cur_y -= (height - 1);
        else
          page->cur_y = 0;
        break;
      }

      case KEY_RESIZE: {
        update = TRUE;
        get_display_size(height, width);
        mvwin(status->win, height, 0);
        mvwin(probe->win, height - 4, 0);

        break;
      }
      case ERR: {
        if (update) {
          update = !update;
          key    = OK;
          if (prevKey == KEY_RESIZE) {
              page->width = width;
              // page->height = blocks;
              wresize(page->win, page->height, page->width);
              // wresize(status->win, status->height, status->width);
              wclear(page->win);
              render_content(page, content, &ref_stack);
          }
        }
        break;
      }
      default:
        key = ERR;
        break;
    }
    if (key != ERR) {
      if (key == ENTER) {
        mdn.bar = probe;
      }
      else {
        if (mdn.bar != status) mdn.bar = status;
        if (height >= page->height)
          wprintw(status->win, "\n%s(ALL) (q to quit)", frame_title[status->frame_type]);
        else if (page->cur_y <= 0)
          wprintw(status->win, "\n%s(TOP) (q to quit)", frame_title[status->frame_type]);
        else if (page->cur_y + height < page->height)
          wprintw(status->win, "\n%s(%d%%) (q to quit)", frame_title[status->frame_type], ((page->cur_y + height) * 100) / page->height);
        else
          wprintw(status->win, "\n%s(END) (q to quit)", frame_title[status->frame_type]);
      }
      pnoutrefresh(page->win, page->cur_y, 0, 0, 0, height - mdn.bar->height, width);
      wnoutrefresh(mdn.bar->win);
    }
    prevKey = key;
    doupdate();
  }
  //  Clean up
  endwin();
  frame_free(page);
  frame_free(status);
  frame_free(probe);
  dom_stack_free(&ref_stack);
  dom_link_free(content);
  xmlFreeDoc(doc);
  return ret;
}

/* vim: set filetype=c: */
