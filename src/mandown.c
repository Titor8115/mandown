#include "mandown.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "view.h"
#include "blender.h"
#include "buffer.h"
#include "markdown.h"

#define READ_UNIT 1024
#define OUTPUT_UNIT 64

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

  /* Reading file */
  in = fopen(file, "r");
  if (!in) {
    error(strerror(errno));
    exit(EXIT_FAILURE);
  }

  ib = bufnew(READ_UNIT);
  bufgrow(ib, READ_UNIT);
  while ((ret = fread(ib->data + ib->size, 1, ib->asize - ib->size, in)) > 0) {
    ib->size += ret;
    bufgrow(ib, ib->size + READ_UNIT);
  }
  fclose(in);

  /* Prepare for nodeHandler */
  ob = bufnew(OUTPUT_UNIT);
  sdblender_renderer(&callbacks, &options, 0);
  markdown = sd_markdown_new(0, 16, &callbacks, &options);
  bufprintf(ob, "<mdn>\n");
  sd_markdown_render(ob, ib->data, ib->size, markdown);
  sd_markdown_free(markdown);
  bufprintf(ob, "</mdn>\n");

  /* Render */
  ret = view(ob, blocks);

  /* Clean up */
  bufrelease(ib);
  bufrelease(ob);
  return ret;
}
