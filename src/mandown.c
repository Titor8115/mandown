#include "mandown.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "blender.h"
#include "buffer.h"
#include "markdown.h"
#include "view.h"

void sd_message(char *output)
{
  fprintf(stdout, "\r%s%snote: %s%s\n", "\033[1m", "\033[36m", "\033[0m",
          output);
}

void sd_error(char *output)
{
  fprintf(stderr, "\r%s%serror: %s%s\n", "\033[1m", "\033[31m", "\033[0m",
          output);
}

void sd_warning(char *output)
{
  fprintf(stderr, "\r%s%swarning: %s%s\n", "\033[1m", "\033[33m", "\033[0m",
          output);
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

Config *
config_new()
{
  Config *ret;
  ret = malloc(sizeof *ret);

  if (ret) {
    ret->mode = PAGE_MODE;
    ret->lineFold = LINE_FOLD;
  }
  return ret;
}

void config_free(Config *configure)
{
  if (!configure)
    return;
  free(configure);
}

int main(int argc, char **argv)
{
  int                      opt;
  int                      ret;
  int                      pfd[2];
  pid_t                    pid;
  char *                   in = NULL;
  char *                   out = NULL;
  FILE *                   fp_in;
  FILE *                   fp_out;
  Config *                 configure;
  struct buf *             ib;
  struct buf *             ob;
  struct sd_callbacks      callbacks;
  struct blender_renderopt options;
  struct sd_markdown *     markdown;

  /* Get current working directory */
  if (argc < 2) {
    usage();
    exit(EXIT_FAILURE);
  }

  configure = config_new();

  while ((opt = getopt(argc, argv, ":f:ho:")) != -1) {
    switch (opt) {
      case 'f':
        in = optarg;
        break;
      case 'h':
        usage();
        exit(EXIT_SUCCESS);
        break;
      case 'o':
        out = optarg;
        configure->mode = FILE_MODE;
        break;
      case ':':
        if (optopt == 'f')
          sderror("No file is given");
        if (optopt == 'o')
          sderror("No path is given for output");
        exit(EXIT_FAILURE);
        break;
      default:
        break;
    }
  }
  for (int i = optind; i < argc; i++)
    in = argv[optind];

  if (!in)
    return EXIT_FAILURE;

  /* Reading file */
  fp_in = fopen(in, "r");
  if (!fp_in) {
    sderror(strerror(errno));
    return EXIT_FAILURE;
  }

  ib = bufnew(READ_UNIT);
  bufgrow(ib, READ_UNIT);
  while ((ret = fread(ib->data + ib->size, 1, ib->asize - ib->size, fp_in)) > 0) {
    ib->size += ret;
    bufgrow(ib, ib->size + READ_UNIT);
  }
  fclose(fp_in);

  /* Prepare for nodeHandler */
  ob = bufnew(OUTPUT_UNIT);
  bufprintf(ob, "<title >%s(7)</title>\n", in);
  sdblender_renderer(&callbacks, &options, 0);
  markdown = sd_markdown_new(0, 16, &callbacks, &options);
  sd_markdown_render(ob, ib->data, ib->size, markdown);
  sd_markdown_free(markdown);

  bufrelease(ib);

  if (configure->mode) {  // * output to file
    if (!(fp_out = fopen(out, "w"))) {
      sderror(strerror(errno));
      return EXIT_FAILURE;
    }
    else
      fwrite((void *)ob->data, ob->size, 0, fp_out);
  }
  else if (!isatty(STDOUT_FILENO)) {  // * output to piped pager
    configure->mode = FILE_MODE;

    pid = pipe(pfd);
    if (pid < 0) {
      perror("pipe failed");
      return EXIT_FAILURE;
    }
    else if (pid == 0) {
      close(pfd[0]);
      dup2(pfd[1], STDOUT_FILENO);
      close(pfd[1]);
    }
    else {
    }
  }
  else {  // * output to mandown pager
    ret = view(configure, ob, blocks);
  }

  /* Clean up */
  bufrelease(ob);
  config_free(configure);
  return ret;
}
