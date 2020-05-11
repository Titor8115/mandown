#include "mandown.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "blender.h"
#include "buffer.h"
#include "markdown.h"
#include "view.h"

#define READ_UNIT 1024
#define OUTPUT_UNIT 64

void message(const char *contents)
{
  fprintf(stdout, "%s%snote: %s%s\n", "\033[1m", "\033[36m", "\033[0m", contents);
}

void error(const char *contents)
{
  fprintf(stderr, "%s%serror: %s%s\n", "\033[1m", "\033[31m", "\033[0m", contents);
}

void warning(const char *contents)
{
  fprintf(stderr, "%s%swarning: %s%s\n", "\033[1m", "\033[33m", "\033[0m", contents);
}

void usage()
{
  fprintf(stdout, "%s\n", "mdn - Markdown Manual, a man(1) like markdown pager");
  fprintf(stdout, "%s\n\n", "Usage: mdn [options...] <filename>");

  fprintf(stdout, "%-20s%s\n", "  -f, --file", "optional flag for filepath");
  fprintf(stdout, "%-20s%s\n\n", "  -h, --help", "this help text");

  fprintf(stdout, "%s\n\n", "Pager control:");

  fprintf(stdout, "%-20s%-10s%s\n", "  Scroll Up:", "\u2191", "  <arrow up>");
  fprintf(stdout, "%-20s%-10s%s\n", "", "BACKSPACE", "<backspace>");
  fprintf(stdout, "%-20s%-10s%s\n", "", "k", "<k>");

  fprintf(stdout, "%-20s%-10s%s\n", "  Scroll Down:", "\u2193", "  <arrow down>");
  fprintf(stdout, "%-20s%-10s%s\n", "", "ENTER", "<enter>");
  fprintf(stdout, "%-20s%-10s%s\n", "", "j", "<j>");

  fprintf(stdout, "%-20s%-10s%s\n\n", "  Exit:", "q", "<q>");
  fprintf(stdout, "%s", "It is still under development. Next featuring HTML render. Looking for co-work buddies!\n");
}

int main(int argc, char **argv)
{
  int opt;
  char *file = NULL;

  int ret, i;
  FILE *in;
  struct buf *ib, *ob;
  struct sd_callbacks callbacks;
  struct blender_renderopt options;
  struct sd_markdown *markdown;

  /* Get current working directory */
  if (argc < 2) {
    usage();
    exit(EXIT_FAILURE);
  }
  else {
    while ((opt = getopt(argc, argv, "hf:")) != -1) {
      switch (opt) {
        case 'f':
          file = optarg;
          break;
        case 'h':
          usage();
          exit(EXIT_SUCCESS);
          // case '?':
          break;
        default:
          break;
      }
    }
    for (i = optind; i < argc; i++) {
      file = argv[optind];
    }
  }

  if (file == NULL) {
    exit(EXIT_FAILURE);
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
  bufprintf(ob, "<article>\n<title >%s(7)</title>", file);
  sdblender_renderer(&callbacks, &options, 0);
  markdown = sd_markdown_new(0, 16, &callbacks, &options);
  sd_markdown_render(ob, ib->data, ib->size, markdown);
  sd_markdown_free(markdown);
  bufprintf(ob, "</article>\n");

  bufrelease(ib);

  /* Render */
  ret = view(ob, blocks);
  // fprintf(stdout, (char *)ob->data);

  /* Clean up */
  bufrelease(ob);

  return ret;
}
