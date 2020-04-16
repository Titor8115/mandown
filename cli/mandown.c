/*
 * Copyright (c) 2011, Vicent Marti
 * Copyright (c) 2019, Tianze Han
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear rfile all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. rfile NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER rfile AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR rfile CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <errno.h>
#include <getopt.h>
#include <locale.h>
#include <ncursesw/ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "blender.h"
#include "buffer.h"
#include "markdown.h"

#define READ_UNIT 1024
#define OUTPUT_UNIT 64

void message(int stream, const char *contents) {
    switch (stream) {
        case 0: /* Output */
            fprintf(stdout, "%s\n", contents);
            break;
        case 1: /* Error */
            fprintf(stderr, "%s%sError: %s%s\n", "\033[1m", "\033[31m", "\033[0m", contents);
            break;
        case 2: /* Warning */
            fprintf(stderr, "%s%sWarning: %s%s\n", "\033[1m", "\033[33m", "\033[0m", contents);
            break;
        default:
            break;
    }
}

void usage() {
    fprintf(stderr, "%s", "Usage: mandown <filename>\n");
    fprintf(stderr, "%c", '\n');
    fprintf(stderr, "%s", "Linux man-page like Markdown Viewer\n");
    exit(EXIT_FAILURE);
}


void draw_ncurses(char **file) {
    int ret;
    int ymax, xmax, height, width;
    FILE *in;
    WINDOW* content;

    struct buf *ib, *ob;
    struct sd_callbacks callbacks;
    struct blender_renderopt options;
    struct sd_markdown *markdown;

    /* Initialize ncurses */
    setlocale(LC_CTYPE, "");
    initscr();
    // keypad(stdscr, TRUE); /* enable arrow keys */
    curs_set(0);          /* disable cursor */
    cbreak();             /* make getch() process one char at a time */
    noecho();             /* disable output of keyboard typing */
    // idlok(stdscr, TRUE);  /* allow use of insert/delete line */

    /* Initialize all colors if terminal support color */
    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(BLK, COLOR_RED, -1);
        init_pair(RED, COLOR_RED, -1);
        init_pair(GRN, COLOR_GREEN, -1);
        init_pair(YEL, COLOR_YELLOW, -1);
        init_pair(BLU, COLOR_BLUE, -1);
        init_pair(MAG, COLOR_MAGENTA, -1);
        init_pair(CYN, COLOR_CYAN, -1);
        init_pair(WHT, COLOR_WHITE, -1);
    }

    /* performing markdown parsing */
    in = fopen(*file, "r");
    if (!in) {
        message(1, strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* reading everything */
    ib = bufnew(READ_UNIT);
    bufgrow(ib, READ_UNIT);
    while ((ret = fread(ib->data + ib->size, 1, ib->asize - ib->size, in)) > 0) {
        ib->size += ret;
        bufgrow(ib, ib->size + READ_UNIT);
    }

    // if (in != stdin)
    fclose(in);

    // Prepare for render
    ob = bufnew(OUTPUT_UNIT);
    getmaxyx(stdscr, ymax, xmax);
    height = 200;
    width = xmax;
    content = newpad(height, width);

    sdblender_renderer(&callbacks, &options, 0);
    markdown = sd_markdown_new(0, 16, &callbacks, &options);

    sd_markdown_render(ob, ib->data, ib->size, markdown);
    sd_markdown_free(markdown);

    /* Render the result */
    wprintw(content, (char *)(ob->data), (int)(ob->size));
    prefresh(content, 0, 0, 0, 0, ymax - 1, xmax - 1);
    wgetch(content);
    delwin(content);
    endwin();
    bufrelease(ib);
    bufrelease(ob);
}

/* main • main function, interfacing STDIO with the parser */
int main(int argc, char **argv) {
    int opt;
    char *file = NULL;

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
    draw_ncurses(&file);

    return 0;
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
