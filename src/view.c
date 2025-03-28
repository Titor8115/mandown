/**
 * Copyright (C) 2019 Tianze Han
 * 
 * This file is part of Mandown.
 * 
 * Mandown is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Mandown is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Mandown.  If not, see <http://www.gnu.org/licenses/>.
 */

#define _XOPEN_SOURCE_EXTENDED 1
#include "view.h"

#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>
#include <libxml/xmlerror.h>
#include <libxml/xmlstring.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "cmd.h"
#include "config.h"

#define cmp_xml(str, node) xmlStrEqual((xmlChar *)str, (xmlChar *)node)
#define get_prop(node, str) xmlGetProp((xmlNode *)node, (xmlChar *)str)

#define get_display_size(y, x) (y = getmaxy(stdscr) - 1, x = getmaxx(stdscr))
#define update_size(f, d)   ((f)->max_y = (d)->max_y, (f)->max_x = (d)->max_x)

/* Available colors: BLACK RED GREEN YELLOW BLUE MAGENTA CYAN WHITE */
static const struct {short fg; short bg;} color_pairs[] =
{
  [C_DEFAULT] = {-1,                  -1},
  [C_RED]     = {COLOR_RED,           -1},
  [C_GREEN]   = {COLOR_GREEN,         -1},
  [C_YELLOW]  = {COLOR_YELLOW,        -1},
  [C_BLUE]    = {COLOR_BLUE,          -1},
  [C_MAGENTA] = {COLOR_MAGENTA,       -1},
  [C_CYAN]    = {COLOR_CYAN,          -1},
};

/*static const struct {char name[19]; attr_t at;} frame_type[] =*/
/*{*/
/*  [FRAME_WIN]  = {"",                 A_NORMAL},*/
/*  [FRAME_PAD]  = {" Markdown page ",  A_NORMAL},*/
/*  [FRAME_SWIN] = {" Markdown page ", A_REVERSE},*/
/*};*/

static const attr_t node_attr[] = {
    [N_EM]    = A_ITALIC,
    [N_PLAIN] = A_NORMAL,
    [N_BOLD]  = A_BOLD,
    [N_INS]   = A_UNDERLINE,
    [N_DEL]   = A_INVIS,
    [N_PRE]   = A_NORMAL,
    [N_CODE]  = COLOR_PAIR(C_YELLOW),
    [N_KBD]   = A_DIM,
    [N_HEAD]  = A_BOLD,
    [N_HREF]  = A_UNDERLINE | COLOR_PAIR(C_BLUE),
};

static void
create_color_pairs()
{
  if (has_colors()) {  /* Set color if terminal support */
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

static inline void
scroll_up(struct frame *page)
{
  if (page->cur_y > 0) page->cur_y--;
}
static inline void
scroll_down(struct frame *page)
{
  int max_y = getmaxy(stdscr) - 1;
  if (page->cur_y + max_y < page->max_y) page->cur_y++;
}

struct frame *
frame_new(int type, WINDOW* parent, int max_y, int max_x, int begin_y, int begin_x)
{
  struct frame *ret;
  ret = malloc(sizeof(struct frame));

  if (ret) {
    if (parent == NULL) {
      if (type == FRAME_WIN)
        ret->win = newwin(max_y, max_x, begin_y, begin_x);
      else if (type == FRAME_PAD)
        ret->win = newpad(max_y, max_x);
      else
        ret->win = NULL;
    }
    else {
      if (type == FRAME_SWIN)
        ret->win = subwin(parent, max_y, max_x, begin_y, begin_x);
      else if (type == FRAME_SPAD)
        ret->win = subpad(parent, max_y, max_x, begin_y, begin_x);
      else
        ret->win = NULL;
    }
    ret->type  = type;
    ret->max_y = max_y;
    ret->max_x = max_x;
    ret->beg_y = begin_y;
    ret->beg_x = begin_x;
    ret->cur_y = 0;
    ret->cur_x = 0;
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

static void
move_cursor(struct frame *pad, int cur_y, int cur_x)
{
  if (!pad->win)
    return;

  while (wmove(pad->win, cur_y, cur_x) == ERR) {
    if (cur_x >= pad->max_x) {
      cur_x = 0;
      cur_y++;
    }

    if (cur_y >= pad->max_y) {
      pad->max_y = cur_y + 2;
    }
    wresize(pad->win, pad->max_y, pad->max_x);
  }
}

static void
render_page(struct frame *dest, struct dom_link *ib, struct stack *href_table)
{
  struct dom_link *      ob;
  struct dom_href_stack *href = NULL;
  char *                 toc = " \n";
  char *                 out;
  char *                 tmp = malloc(sizeof(char) * OUTPUT_UNIT);
  size_t                 stack_pos = 0;
  size_t                 position;
  int                    cur_y = 0;
  int                    cur_x = 0;
  int                    len;

  for (ob = ib; ob != NULL; ob = ob->next) {
    if (ob->prop & P_CONTROL) {
      if (ob->prop & P_BEG_A) {
        wattron(dest->win, ob->attr);
        if ((ob->prop & P_PREFIXED))
          toc = "\n";
        if (ob->prop & P_HYPERLINK) {
          href = href_table->item[stack_pos];
          stack_pos++;
          href->beg_y = cur_y;
          href->beg_x = cur_x;
        }
      }
      else {
        wattroff(dest->win, ob->attr);
        if ((ob->prop & P_PREFIXED))
          toc = " \n";
      }
      if (ob->prop & P_SPLIT) {
        move_cursor(dest, ++cur_y, cur_x = ob->indent);
      }
      if (ob->prop & P_SECTION) {
        move_cursor(dest, ++cur_y, cur_x = ob->indent);
      }
    }
    if (!ob->buf)
      continue;
    else {
      tmp = realloc(tmp, ob->buf->size + 1);
      memcpy(tmp, ob->buf->data, ob->buf->size + 1);
    }

    position = 0;
    out      = strtok(tmp, toc);
    while (out != NULL) {
      len = strlen(out);

      if (cur_x == 0) move_cursor(dest, cur_y, cur_x = ob->indent);

      if ((cur_x > ob->indent) && (cur_x + len >= dest->max_x)) {
        cur_x = ob->indent;
        cur_y++;
        move_cursor(dest, cur_y + (len + ob->indent) / dest->max_x, cur_x);
      }
      waddnstr(dest->win, out, len);
      getyx(dest->win, cur_y, cur_x);
      if (href != NULL) {
        href->end_y = cur_y;
        href->end_x = cur_x;
        href = NULL;
      }
      // j += len;
      position += len;
      if (position < ob->buf->size) {
        if (toc[0] == '\n') {
          // cur_x = ob->indent;
          move_cursor(dest, ++cur_y, cur_x = 0);
        }
        else if ((toc[0] == ' ') && (cur_x < dest->max_x)) {
          cur_x++;
          waddch(dest->win, toc[0]);
        }
      }

      position++;

      out = strtok(NULL, toc);
    }
  }
  free(tmp);

  if (dest->max_y != cur_y) {
    dest->max_y = cur_y + 1;
    wresize(dest->win, dest->max_y, dest->max_x);
  }
}

struct dom_link *
content_setup(xmlNode *node, int indent, struct stack *href_table)
{
  struct dom_href_stack *href;
  struct dom_link *      link;
  struct dom_link *      head = NULL;
  struct dom_link *      tail = NULL;
  htmlNodePtr            cur_node;
  xmlChar *              xml_prop;

  for (cur_node = node; cur_node != NULL; cur_node = cur_node->next) {
    link         = dom_link_new(bufnew(OUTPUT_UNIT));
    link->indent = indent;
    if (cur_node->type == XML_ELEMENT_NODE) {
      tail       = dom_link_new(NULL);
      link->prop = P_CONTROL | P_BEG_A;
      tail->prop = P_CONTROL | P_END_A;
      // if (cmp_xml("html", cur_node->name)) {
      // }
      // else if (cmp_xml("head", cur_node->name)) {
      // }
      // else if (cmp_xml("body", cur_node->name)) {
      // }
      if (cmp_xml("body", cur_node->parent->name)) {
        link->prop |= P_SPLIT;
      }

      if (cmp_xml("title", cur_node->name)) {
        link->indent = 0;
        link->attr |= node_attr[N_INS];
        // tail->prop |= P_SPLIT;
      }
      else if (cmp_xml("h1", cur_node->name)) {  /* h1 as "NAME" .SH */
        bufputs(link->buf, "NAME: ");
        link->indent = 0;
        link->prop |= P_SECTION;
        link->attr |= node_attr[N_HEAD];
      }
      else if (cmp_xml("h2", cur_node->name)) {  /* h2 for all other .SH */
        link->indent = 0;
        link->prop |= P_SECTION;
        link->attr |= node_attr[N_HEAD];
      }
      else if (cmp_xml("h3", cur_node->name)) {  /* h3 for .SS */
        link->indent = 3;
        link->prop |= P_SECTION;
        link->attr |= node_attr[N_HEAD];
      }
      else if (cmp_xml("h4", cur_node->name)) {  /* h4 as Sub of .SS */
        /*bufputs(link->buf, "SC: ");*/
        link->indent = 4;
        link->prop |= P_SECTION;
        link->attr |= node_attr[N_HEAD] | node_attr[N_EM];
      }
      else if (cmp_xml("h5", cur_node->name)) {  /* h5 as Points in Sub */
        /*bufputs(link->buf, "PT: ");*/
        link->indent = 5;
        link->prop |= P_SECTION;
        link->attr |= node_attr[N_HEAD] | node_attr[N_INS];
      }
      else if (cmp_xml("h6", cur_node->name)) {  /* h6 as Sub of Points */
        /*bufputs(link->buf, "PS: ");*/
        link->indent = 6;
        link->prop |= P_SECTION;
        link->attr |= node_attr[N_HEAD] | node_attr[N_INS] | node_attr[N_EM];
      }
      else if (cmp_xml("ul", cur_node->name) ||
               cmp_xml("ol", cur_node->name)) {  /* unordered list */
        link->indent += 2;
      }
      else if (cmp_xml("li", cur_node->name)) {  /* list item */
        bufputs(link->buf, "• ");
        link->prop |= P_SPLIT;
      }
      else if (cmp_xml("p", cur_node->name)) {  /* paragraph */
        link->attr |= node_attr[N_PLAIN];
      }
      else if (cmp_xml("strong", cur_node->name) ||
               cmp_xml("b", cur_node->name)) {  /* bold */
        link->attr |= node_attr[N_BOLD];
      }
      else if (cmp_xml("em", cur_node->name) ||
               cmp_xml("i", cur_node->name)) {  /* italic */
        link->attr |= node_attr[N_EM];
      }
      else if (cmp_xml("ins", cur_node->name) ||
               cmp_xml("u", cur_node->name)) {  /* underline */
        link->attr |= node_attr[N_INS];
      }
      else if (cmp_xml("del", cur_node->name) ||
               cmp_xml("s", cur_node->name)) {  /* strikethrough */
        link->attr |= node_attr[N_DEL];
      }
      /*else if (cmp_xml("kbd", cur_node->name)) {   keyboard key */
      /*  link->attr |= node_attr[N_KBD];*/
      /*}*/
      else if (cmp_xml("pre", cur_node->name)) {  /* codeblock */
        link->prop |= P_PREFIXED;
        tail->prop |= P_PREFIXED;
        link->indent += 2;
      }
      else if (cmp_xml("code", cur_node->name)) {  /* codeblock */
        link->attr |= node_attr[N_CODE];
      }
      else if (cmp_xml("a", cur_node->name)) {  /* hyperlink */
        link->attr |= node_attr[N_HREF];
        link->prop |= P_HYPERLINK | P_PREFIXED;
        tail->prop |= P_HYPERLINK | P_PREFIXED;
        xml_prop = get_prop(cur_node, "href");
        if (xml_prop != NULL) {
          href = dom_href_new(bufnew(xmlStrlen(xml_prop)));
          href->index = href_table->size;
          bufputs(href->url, (char *)xml_prop);
          bufcstr(href->url);
          stack_push(href_table, href);
        }
        xmlFree(xml_prop);
      }
      else if (cmp_xml("img", cur_node->name)) {  /* image */
        link->attr |= node_attr[N_HREF];
        link->prop |= P_HYPERLINK | P_PREFIXED;
        tail->prop |= P_HYPERLINK | P_PREFIXED;
        xml_prop = get_prop(cur_node, "alt");
        if (xml_prop != NULL) {
          bufputs(link->buf, (char *)xml_prop);
        }
        xml_prop = get_prop(cur_node, "src");
        if (xml_prop != NULL) {
          href = dom_href_new(bufnew(xmlStrlen(xml_prop)));
          href->index = href_table->size;
          bufputs(href->url, (char *)xml_prop);
          bufcstr(href->url);
          stack_push(href_table, href);
        }
        xmlFree(xml_prop);
      }

    }
    else if (cur_node->type == XML_TEXT_NODE) {
      link->prop = P_REGULAR;
      bufputs(link->buf, (char *)cur_node->content);
    }

    if (link->buf->size == 0) {
      bufrelease(link->buf);
      link->buf = NULL;
    }
    else
      bufcstr(link->buf);
    link->next = content_setup(cur_node->children, link->indent, href_table);
    dom_link_append(&head, link);

    if (tail) {
      tail->attr = link->attr;
      dom_link_append(&head, tail);
      tail = tail->next;
    }
  }

  return head;
}


int view(const struct buf *ob, int href_count)
{
  struct frame *         page;   /* content container */
  struct frame *         status; /* status line container */
  struct frame *         probe;
  struct dom_href_stack *link = NULL;
  struct dom_link *      content;
  struct mdn_cfg *       config;
  struct stack           ref_stack;
  htmlDocPtr             doc;
  htmlNodePtr            rootNode;
  MEVENT                 event;
  mdn_command            cmd;               /* command enum */
  mdn_command            prev_cmd = CMD_OK; /* previous command */
  int                    update = TRUE;     /* control needs to update */
  int                    stack_index = -1;
  mmask_t                mouse = 0;
  int                    height;
  int                    width;
  int                    cur_y;
  int                    cur_x;
  
  LIBXML_TEST_VERSION;
  /* Grow DOM tree; */
  doc = htmlReadMemory((char *)(ob->data), (int)(ob->size),
                       "file", NULL, HTML_PARSE_NOBLANKS | HTML_PARSE_NOWARNING | HTML_PARSE_RECOVER | HTML_PARSE_NOERROR);
  if (!doc) {
    sderror("Cannot make sense of given file in HTML/xHTML format");
    return EXIT_FAILURE;
  }

  rootNode = xmlDocGetRootElement(doc);
  if (!rootNode) {
    xmlFreeDoc(doc);
    sderror("Empty document, or no Root Element");
    return EXIT_FAILURE;
  }

  config = configure();

  dom_stack_new(&ref_stack, href_count);
  content = content_setup(rootNode, config->indent, &ref_stack);

  setlocale(LC_ALL, "");
  newterm(NULL, stdout, stderr);
  noecho();             /* disable echo of keyboard typing */
  keypad(stdscr, TRUE); /* enable arrow keys */
  timeout(200);
  cbreak();
  if (config->use_mouse)
    mouse = mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);

  create_color_pairs();

  get_display_size(height, width);

  page   = frame_new(FRAME_PAD, NULL, (int)ob->size / width, width, 0, 0);
  status = frame_new(FRAME_WIN, NULL, 1, width, height, 0);
  probe = frame_new(FRAME_WIN, NULL, 4, width, height - 4, 0);
  wattrset(status->win, A_REVERSE);
  wattrset(probe->win, A_NORMAL);

  render_page(page, content, &ref_stack);

  refresh();


  while (TRUE) {
    switch ((config->control_scheme)[0]) {
      case 'M':
      case 'm': {
        cmd = mdn_cmd_scheme(getch());
        break;
      }
      case 'L':
      case 'l': {
        cmd = less_cmd_scheme(getch());
        break;
      }
      case 'V':
      case 'v': {
        cmd = vim_cmd_scheme(getch());
        break;
      }
      default:
        cmd = mdn_cmd_scheme(getch());
        break;
    }

    if (cmd == CMD_EXIT) {
        break;
    }

    if ((cmd == CMD_MOUSE_TOGGLE) && config->use_mouse) {
      if (mouse == 0)
        mouse = mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, &event.bstate);
      else
        mouse = mousemask(0, &event.bstate);
    }
      if (cmd == CMD_MOUSE_EVENT) {
        if (getmouse(&event) == OK) {
          if (event.bstate & BUTTON4_PRESSED)
            cmd = CMD_MOVE_UP;
          else if (event.bstate & BUTTON5_PRESSED)
            cmd = CMD_MOVE_DOWN;
          else if ((event.bstate & BUTTON1_DOUBLE_CLICKED)) {
            link = dom_stack_find(&ref_stack, 0, page->cur_y, page->cur_y + height, page->cur_y + event.y, event.x);

            if (link != NULL) {
              cur_y = event.y;
              cur_x = event.x;
              move(cur_y, cur_x);
              cmd = CMD_COMFIRM;
            }
          }
        }
      }
  
      switch (cmd) {
        case CMD_SELECT_HREF: {
          link = dom_stack_find(&ref_stack, stack_index, page->cur_y, page->cur_y + height, -1, -1);

          if (link != NULL) {
            stack_index = link->index;
            cur_y = link->end_y - page->cur_y;
            cur_x = link->end_x - page->cur_x;
            move(cur_y, cur_x);
          }
        break;
      }
      case CMD_COMFIRM: {
        werase(probe->win);
        box(probe->win, ' ', 0);
        wattron(probe->win, A_REVERSE);
        wmove(probe->win, probe->cur_y, probe->cur_x + 2);
        waddstr(probe->win, " Reference ");
        wattroff(probe->win, A_REVERSE);

        if (link != NULL) {
          wmove(probe->win, probe->cur_y + 1, probe->cur_x);
          waddnstr(probe->win, (char *)link->url->data, (int)link->url->size);
        }
        break;
      }
      case CMD_MOVE_DOWN: {
        scroll_down(page);
        break;
      }
      case CMD_MOVE_UP: {
        scroll_up(page);
        break;
      }
      case CMD_MOVE_NPAGE: {
        if (page->cur_y + ((height - 1) << 1) < page->max_y)
          page->cur_y += (height - 1);
        else
          page->cur_y = page->max_y - height;
        break;
      }
      case CMD_MOVE_PPAGE: {
        if (page->cur_y - (height - 1) > 0)
          page->cur_y -= (height - 1);
        else
          page->cur_y = 0;
        break;
      }
      case CMD_MOVE_TOP: {
        page->cur_y = 0;
        break;
      }
      case CMD_MOVE_EOF: {
        page->cur_y = page->max_y - height;
        break;
      }
      case CMD_RESIZE_EVENT: {
        update = TRUE;
        get_display_size(height, width);
        mvwin(status->win, height, 0);
        mvwin(probe->win, height - 4, 0);
        break;
      }
      case CMD_ERR: {
        if (update) {
          update = !update;
          cmd    = CMD_OK;
          if (prev_cmd == CMD_RESIZE_EVENT) {
              page->max_x = width;
              wresize(page->win, page->max_y, page->max_x);
              wclear(page->win);
              render_page(page, content, &ref_stack);
          }
        }
        break;
      }
      default:
        cmd = CMD_ERR;
        break;
    }

    if (cmd != CMD_ERR) {
      if (cmd == CMD_COMFIRM) {
        pnoutrefresh(page->win, page->cur_y, 0, 0, 0, height - probe->max_y, width);

        wnoutrefresh(probe->win);
      }
      else {
        werase(status->win);
        if (height >= page->max_y)
          waddstr(status->win, " Markdown page (ALL) (q to quit)");
        else if (page->cur_y <= 0)
          waddstr(status->win, " Markdown page (TOP) (q to quit)");
        else if (page->cur_y + height < page->max_y)
          wprintw(status->win, " Markdown page (%d%%) (q to quit)", ((page->cur_y + height) * 100) / page->max_y);
        else
          waddstr(status->win, " Markdown page (END) (q to quit)");
        pnoutrefresh(page->win, page->cur_y, 0, 0, 0, height - status->max_y, width);

        wnoutrefresh(status->win);
      }
    }
    prev_cmd = cmd;
    doupdate();
  }

  /* Clean up */
  endwin();
  frame_free(page);
  frame_free(status);
  frame_free(probe);
  dom_stack_free(&ref_stack);
  dom_link_free(content);
  xmlFreeDoc(doc);
  return EXIT_SUCCESS;
}

/* vim: set filetype=c: */
