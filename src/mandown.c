#include "mandown.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG
#include <unistd.h>
#endif

#include "blender.h"
#include "markdown.h"

void sd_info(char *output)
{
  fprintf(stdout, "%snote: %s%s\n", "\033[36m", "\033[0m", output);
}

void sd_error(int *ret, char *output)
{
  (*ret) = 1;
  fprintf(stderr, "%serror: %s%s\n", "\033[31m", "\033[0m", output);
}

void sd_warn(char *output)
{
  fprintf(stderr, "%swarning: %s%s\n", "\033[33m", "\033[0m", output);
}

void usage()
{
  fprintf(stdout, "%s\n", "mdn - Markdown Manual, a man(1) like markdown pager");
  fprintf(stdout, "%s\n\n", "Usage: mdn <title> [options...]");

  fprintf(stdout, "%-20s%s\n", "  -f, --file", "optional flag for filepath");
  fprintf(stdout, "%-20s%s\n", "  -h, --help", "this help text");
  fprintf(stdout, "%-20s%s\n\n", "  -o, --outpath", "xhtml version of input file");

  fprintf(stdout, "%s\n\n", "Pager control:");

  fprintf(stdout, "%-20s%-15s%s\n", "  Scroll Up:", "\u2191", "  <arrow up>");
  fprintf(stdout, "%-20s%-15s%s\n\n", "", "k", "<k>");

  fprintf(stdout, "%-20s%-15s%s\n", "  Scroll Down:", "\u2193", "  <arrow down>");
  fprintf(stdout, "%-20s%-15s%s\n\n", "", "j", "<j>");

  fprintf(stdout, "%-20s%-15s%s\n", "  Page Up:", "fn + \u2191", "<function + arrow up>");
  fprintf(stdout, "%-20s%-15s%s\n", "", "BACKSPACE", "<back space>");
  fprintf(stdout, "%-20s%-15s%s\n\n", "", "pg up", "<page up>");

  fprintf(stdout, "%-20s%-15s%s\n", "  Page Down:", "fn + \u2193", "<function + arrow down>");
  fprintf(stdout, "%-20s%-15s%s\n", "", "SPACE", "<space bar>");
  fprintf(stdout, "%-20s%-15s%s\n\n", "", "pg dn", "<page down>");

  fprintf(stdout, "%-20s%-15s%s\n", "  Select href:", "TAB", "<tab>");
  fprintf(stdout, "%-20s%-15s%s\n", "  Get href link:", "ENTER", "<enter>");

  fprintf(stdout, "%-20s%-15s%s\n\n", "  Exit:", "q", "<q>");
  fprintf(stdout, "%s", "HTML format is partly supported. TUI help page soon replace Pager control section\n");
}

static struct config setting = {
    PAGE_MODE,
    LINE_FOLD,
};

struct config *
config_new()
{
  struct config *ret;
  ret = malloc(sizeof *ret);

  if (ret) {
    ret->mode = PAGE_MODE;
    ret->fold = LINE_FOLD;
  }
  return ret;
}

void config_free(struct config *config)
{
  if (!config)
    return;
  free(config);
}

static const char *
get_file_ext(const char *file, const char ext)
{
  const char *dot = strrchr(file, ext);
  if (!dot || dot == file) return "";
  return dot + 1;
}

int main(int argc, char **argv)
{
#ifdef DEBUG

  int   pfd[2];
  pid_t pid;

#endif
  int          ret = 0;
  int          opt;
  unsigned int extensions = MKDEXT_NO_INTRA_EMPHASIS |
                             MKDEXT_TABLES |
                             MKDEXT_AUTOLINK |
                             MKDEXT_STRIKETHROUGH;
  const char *             in  = NULL;
  const char *             out = NULL;
  const char *             ext;
  const char *             title;
  FILE *                   fp_in;
  FILE *                   fp_out;
  struct buf *             ib;
  struct buf *             ob;
  struct sd_markdown *     markdown;
  struct sd_callbacks      callbacks;
  struct blender_renderopt options;

  /* Get current working directory */
  if (argc < 2) {
    usage();
    return EXIT_FAILURE;
  }

  // config = config_new();

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
        out          = optarg;
        setting.mode = FILE_MODE;
        break;
      case ':':
        if (optopt == 'f') {
          sderror(&ret, "No file is given");
          return ret;
        }
        if (optopt == 'o') {
          out          = "/dev/stdout";
          setting.mode = FILE_MODE;
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
    sderror(&ret, "No file is given");
    return ret;
  }
    /* Reading file */
    fp_in = fopen(in, "r+");
  if (!fp_in) {
    sderror(&ret, strerror(errno));
    return ret;
  }

  ib = bufnew(READ_UNIT);
  bufgrow(ib, READ_UNIT);
  while ((ret = fread(ib->data + ib->size, 1, ib->asize - ib->size, fp_in)) > 0) {
    ib->size += ret;
    bufgrow(ib, ib->size + READ_UNIT);
  }
  fclose(fp_in);

  title = get_file_ext(in, '/');
  ext   = get_file_ext(in, '.');
  if (((strncmp(ext, "", 1)) == 0) || (strcmp(title, "")) == 0) {
    title = in;
  }
  else if ((strcmp(ext, "html")) == 0) {
    ob = ib;
    goto prepare_render;
  }
  ob = bufnew(OUTPUT_UNIT);
  sdblender_renderer(&callbacks, &options, 0);
  bufprintf(ob, "<html><head><title >%s(7)</title></head><body>", in);
  markdown = sd_markdown_new(extensions, 16, &callbacks, &options);
  sd_markdown_render(ob, ib->data, ib->size, markdown);
  bufputs(ob, "</body></html>\n");
  sd_markdown_free(markdown);
  bufrelease(ib);

/* Prepare for parsing */
prepare_render:
  if (setting.mode) {  // * output to file
    if (!(fp_out = fopen(out, "w"))) {
      sderror(&ret, strerror(errno));
      goto clean_up;
    }
    else {
      fwrite((void *)ob->data, ob->size, 1, fp_out);
      fclose(fp_out);
    }
  }
#ifdef DEBUG

  else if (!isatty(STDOUT_FILENO)) {  // * output to piped pager
    config.mode = FILE_MODE;

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

#endif

  else {  // * output to mandown pager
    ret = view(&setting, ob, href);
  }

  /* Clean up */
clean_up:
  bufrelease(ob);
  // config_free(config);
  return ret;
}
