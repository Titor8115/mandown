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

void sd_info(char *output)
{
  fprintf(stdout, "%snote: %s%s\n", "\033[36m", "\033[0m", output);
}

void sd_error(char *output)
{
  fprintf(stderr, "%serror: %s%s\n", "\033[31m", "\033[0m", output);
}

void sd_warn(char *output)
{
  fprintf(stderr, "%swarning: %s%s\n", "\033[33m", "\033[0m", output);
}

void usage()
{
  fprintf(stdout, "%s\n", "mdn - Markdown Manual, a man(1) like markdown pager");
  fprintf(stdout, "%s\n\n", "Usage: mdn <filename> [options...]");

  fprintf(stdout, "%-20s%s\n", "  -f, --file", "optional flag for filepath");
  fprintf(stdout, "%-20s%s\n", "  -h, --help", "this help text");
  fprintf(stdout, "%-20s%s\n\n", "  -o, --outpath", "xhtml version of input file");

  fprintf(stdout, "%s\n\n", "Pager control:");

  fprintf(stdout, "%-20s%-15s%s\n", "  Scroll Up:", "\u2191", "  <arrow up>");
  fprintf(stdout, "%-20s%-15s%s\n", "", "BACKSPACE", "<backspace>");
  fprintf(stdout, "%-20s%-15s%s\n\n", "", "k", "<k>");

  fprintf(stdout, "%-20s%-15s%s\n", "  Scroll Down:", "\u2193", "  <arrow down>");
  fprintf(stdout, "%-20s%-15s%s\n", "", "ENTER", "<enter>");
  fprintf(stdout, "%-20s%-15s%s\n\n", "", "j", "<j>");

  fprintf(stdout, "%-20s%-15s%s\n", "  Page Up:", "fn + \u2191", "<function + arrow up>");
  fprintf(stdout, "%-20s%-15s%s\n", "", "BACKSPACE", "<back space>");
  fprintf(stdout, "%-20s%-15s%s\n\n", "", "pg up", "<page up>");

  fprintf(stdout, "%-20s%-15s%s\n", "  Page Down:", "fn + \u2193", "<function + arrow down>");
  fprintf(stdout, "%-20s%-15s%s\n", "", "SPACE", "<space bar>");
  fprintf(stdout, "%-20s%-15s%s\n\n", "", "pg dn", "<page down>");

  fprintf(stdout, "%-20s%-15s%s\n\n", "  Exit:", "q", "<q>");
  fprintf(stdout, "%s", "HTML format is partly supported. TUI help page soon replace Pager control section\n");
}

Config *
config_new()
{
  Config *ret;
  ret = malloc(sizeof *ret);

  if (ret) {
    ret->mode     = PAGE_MODE;
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
  unsigned int             extensions = (MKDEXT_NO_INTRA_EMPHASIS | MKDEXT_TABLES |
                                          MKDEXT_AUTOLINK | MKDEXT_STRIKETHROUGH);
  pid_t                    pid;
  char *                   in  = NULL;
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
    return EXIT_FAILURE;
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
        out             = optarg;
        configure->mode = FILE_MODE;
        break;
      case ':':
        if (optopt == 'f')
          sderror("No file is given");
        if (optopt == 'o') {
          out = "/dev/stdout";
          configure->mode = FILE_MODE;
        }
        break;
      default:
        break;
    }
  }

  if (!in) {
    for (int i = optind; i < argc; i++)
      in = argv[optind];
  }
  if (!in) {
    sderror("No file is given");
    return EXIT_FAILURE;
  }

  /* Reading file */
  fp_in = fopen(in, "r+");
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
  markdown = sd_markdown_new(extensions, 16, &callbacks, &options);
  sd_markdown_render(ob, ib->data, ib->size, markdown);
  sd_markdown_free(markdown);

  bufrelease(ib);


  if (configure->mode) {  // * output to file
    if (!(fp_out = fopen(out, "w"))) {
      sderror(strerror(errno));
      return EXIT_FAILURE;
    }
    else {
      fwrite((void *)ob->data, ob->size, 1, fp_out);
      fclose(fp_out);
    }
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
    ret = view(configure, ob, href);
  }

  /* Clean up */
  bufrelease(ob);
  config_free(configure);
  return ret;
}
