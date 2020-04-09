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
#include <sys/stat.h>
#include <unistd.h>

#include "blender.h"
#include "buffer.h"
#include "markdown.h"

#define READ_UNIT 1024
#define OUTPUT_UNIT 64
#define PATH_MAX 1024

static void message(int stream, const char *contents) {
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

int draw_ncurses(char **file) {
    int ret;
    // int xmax, ymax;
    struct buf *ib, *ob;
    FILE *in;

    struct sd_callbacks callbacks;
    struct blender_renderopt options;
    struct sd_markdown *markdown;


    in = fopen(*file, "r");
    if (!in) {
        message(1, strerror(errno));
        return 1;
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

    /* performing markdown parsing */
    ob = bufnew(OUTPUT_UNIT);

    sdblender_renderer(&callbacks, &options, 0);
    markdown = sd_markdown_new(0, 16, &callbacks, &options);


    sd_markdown_render(ob, ib->data, ib->size, markdown);
    sd_markdown_free(markdown);

    /* Initialize ncurses */
    initscr();
    keypad(stdscr, TRUE); /* enable arrow keys */
    // curs_set(0);          /* disable cursor */
    cbreak();             /* make getch() process one char at a time */
    noecho();             /* disable output of keyboard typing */
    // idlok(stdscr, TRUE);  /* allow use of insert/delete line */

    /* Initialize all colors if terminal support color */
    // if (has_colors()) {
    //   start_color();
    //   // use_default_colors();
    //   init_pair(BLK, COLOR_WHITE, COLOR_BLACK);
    //   init_pair(RED, COLOR_RED, COLOR_BLACK);
    //   init_pair(GRN, COLOR_GREEN, COLOR_BLACK);
    //   init_pair(YEL, COLOR_YELLOW, COLOR_BLACK);
    //   init_pair(BLU, COLOR_BLUE, COLOR_BLACK);
    //   init_pair(MAG, COLOR_MAGENTA, COLOR_BLACK);
    //   init_pair(CYN, COLOR_CYAN, COLOR_BLACK);
    //   init_pair(WHT, COLOR_WHITE, COLOR_BLACK);
    // }


    // WINDOW *content = newpad(ymax, xmax);

    /* writing the result to stdout */
    // ret = fwrite(ob->data, 1, ob->size, stdout);
    printw("%s", (char *)(ob->data));
    printw("%zu ", ob->nline);
    // getyx(stdscr, ymax, xmax);
    // printw("y:%d x:%d\n", ymax, xmax);
    // printw("%zu\n", ob->size);

    refresh();
    /* cleanup */
    bufrelease(ib);
    bufrelease(ob);
    sleep(10);
    endwin();

    return (ret < 0) ? -1 : 0;
}

void usage() {
    fprintf(stderr, "%s", "Usage: mandown <filename>\n");
    fprintf(stderr, "%c", '\n');
    fprintf(stderr, "%s", "Linux man-page like Markdown Viewer\n");
    exit(EXIT_FAILURE);
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
            case 'f': file = optarg; break;
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
    /* opening the file if given from the command line */
    setlocale(LC_CTYPE, "");

    draw_ncurses(&file);

    return 0;
}

/* vim: set filetype=c: */
