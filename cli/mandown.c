#include "mandown.h"

#include <errno.h>
#include <getopt.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <locale.h>
#include <ncursesw/ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "blender.h"
#include "buffer.h"
#include "markdown.h"

void message(const char *contents) {
  fprintf(stdout, "%s\n", contents);
}

void error(const char *contents) {
  fprintf(stderr, "%s%sError: %s%s\n", "\033[1m", "\033[31m", "\033[0m", contents);
}

void warning(const char *contents) {
  fprintf(stderr, "%s%sWarning: %s%s\n", "\033[1m", "\033[33m", "\033[0m", contents);
}

void usage() {
  fprintf(stderr, "%s", "Usage: mandown <filename>\n");
  fprintf(stderr, "%c", '\n');
  fprintf(stderr, "%s", "Linux man-page like Markdown Viewer\n");
  exit(EXIT_FAILURE);
}

void traverse_tree(xmlNode *a_node, WINDOW *dest) {
  xmlNode *cur_node;

  for (cur_node = a_node; cur_node != NULL; cur_node = cur_node->next) {
    // if (cur_node->name == "h1") {
    //   wprintw(dest, "%s(7README)\n\n", cur_node->content);
    //   wprintw(dest, "NAME\n\t%s", cur_node->content);
    // }
    if (cur_node->type == XML_TEXT_NODE) {
      wprintw(dest, "%s\n", cur_node->content);
    }
    traverse_tree(cur_node->children, dest);
    }
}

int draw_ncurses(struct buf *ob) {
  int ymax, xmax, height, width;
  WINDOW *content;
  xmlDoc *doc;
  xmlNode *root_node;

  LIBXML_TEST_VERSION;

  doc = xmlReadMemory((char *)(ob->data), (int)(ob->size), "noname.xml", NULL, XML_PARSE_NOBLANKS);
  if (doc == NULL) {
    // error(strerror(errno));
    error("Failed to parse document\n");
    return 1;
  }

  root_node = xmlDocGetRootElement(doc);
  if (root_node == NULL) {
    error("empty document\n");
    xmlFreeDoc(doc);
    return 1;
  }

  /* Initialize ncurses */
  setlocale(LC_CTYPE, "");
  initscr();
  // keypad(stdscr, TRUE); /* enable arrow keys */
  curs_set(0); /* disable cursor */
  cbreak();    /* make getch() process one char at a time */
  noecho();    /* disable output of keyboard typing */
  // idlok(stdscr, TRUE);  /* allow use of insert/delete line */

  /* Initialize all colors if terminal support color */
  // if (has_colors()) {
  //   start_color();
  //   use_default_colors();
  //   init_pair(BLK, COLOR_RED, -1);
  //   init_pair(RED, COLOR_RED, -1);
  //   init_pair(GRN, COLOR_GREEN, -1);
  //   init_pair(YEL, COLOR_YELLOW, -1);
  //   init_pair(BLU, COLOR_BLUE, -1);
  //   init_pair(MAG, COLOR_MAGENTA, -1);
  //   init_pair(CYN, COLOR_CYAN, -1);
  //   init_pair(WHT, COLOR_WHITE, -1);
  // }

  getmaxyx(stdscr, ymax, xmax);
  height = blocks + 1;
  width = xmax;
  content = newpad(height, width);
  // content_info = newwin(1, )

  /* Render the result */
  traverse_tree(root_node, content);
  // wprintw(content, (char *)(ob->data), (int)(ob->size));
  prefresh(content, 0, 0, 0, 0, ymax - 2, xmax);
  wgetch(content);

  /* Clean up */
  xmlFreeDoc(doc);
  delwin(content);
  endwin();
  return 0;
}

int main(int argc, char **argv) {
  int opt;
  char *file = NULL;

  int ret;
  FILE *in;
  struct buf *ib, *ob;
  struct sd_callbacks callbacks;
  struct blender_renderopt options;
  struct sd_markdown *markdown;

  /* Get current working directory */
  if (argc < 2) {
    usage();
  } else if (argc == 2) {
    file = argv[1];
  } else {
    while ((opt = getopt(argc, argv, "f:")) != -1) {
      switch (opt) {
        case 'f':
          file = optarg;
          break;
        case ':':
          fprintf(stderr, "%s: -'%c' needs an argument\n", argv[0], optopt);
          usage();
          break;
        case '?':
        default:
          fprintf(stderr, "%s: Unknown option -'%c'\n", argv[0], optopt);
          usage();
          break;
      }
    }
  }

  /********************
   * Markdown parsing *
   ********************/
  in = fopen(file, "r");
  if (!in) {
    error(strerror(errno));
    exit(EXIT_FAILURE);
  }

  /* Reading everything */
  ib = bufnew(READ_UNIT);
  bufgrow(ib, READ_UNIT);
  while ((ret = fread(ib->data + ib->size, 1, ib->asize - ib->size, in)) > 0) {
    ib->size += ret;
    bufgrow(ib, ib->size + READ_UNIT);
  }
  fclose(in);

  /* Prepare for render */
  ob = bufnew(OUTPUT_UNIT);
  sdblender_renderer(&callbacks, &options, 0);
  markdown = sd_markdown_new(0, 16, &callbacks, &options);
  bufprintf(ob, "<mdn>\n");
  sd_markdown_render(ob, ib->data, ib->size, markdown);
  sd_markdown_free(markdown);
  bufprintf(ob, "</mdn>\n");

  /* Render */
  ret = draw_ncurses(ob);

  /* Clean up */
  bufrelease(ib);
  bufrelease(ob);
  return ret;
}

// int newlines = 0, Choice = 0, Key = 0;
// for (int i = 0; i < ob->size; i++)
//     if (ob->data[i] == '\n') newlines++;
// int PadHeight = ((ob->size - newlines) / Width + newlines + 1);
// mypad = newpad(PadHeight, Width);
// keypad(content, true);
// waddwstr(content, c_str());
// refresh();
// int cols = 0;
// while ((Key = wgetch(content)) != 'q') {
//     prefresh(content, cols, 0, 0, 0, ymax, xmax);
//     switch (Key) {
//         case KEY_UP: {
//             if (cols <= 0) continue;
//             cols--;
//             break;
//         }
//         case KEY_DOWN: {
//             if (cols + ymax + 1 >= PadHeight) continue;
//             cols++;
//             break;
//         }
//         case KEY_PPAGE: /* Page Up */
//         {
//             if (cols <= 0) continue;
//             cols -= xmax;
//             if (cols < 0) cols = 0;
//             break;
//         }
//         case KEY_NPAGE: /* Page Down */
//             if (cols + ymax + 1 >= PadHeight) continue;
//             cols += Height;
//             if (cols + ymax + 1 > PadHeight) cols = PadHeight - Height - 1;
//             break;
//         case KEY_HOME:
//             cols = 0;
//             break;
//         case KEY_END:
//             cols = PadHeight - Height - 1;
//             break;
//         case 10: /* Enter */
//         {
//             Choice = 1;
//             break;
//         }
//     }
//     refresh();
// }

/* vim: set filetype=c: */
