#include "mandown.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
  int          mode = 0;
  unsigned int extensions = MKDEXT_NO_INTRA_EMPHASIS |
                             MKDEXT_TABLES |
                             MKDEXT_AUTOLINK |
                             MKDEXT_STRIKETHROUGH;
  const char *             in  = NULL;
  const char *             out = NULL;
  const char *             ext = "";
  const char *             title = "";
  FILE *                   fp_in;
  FILE *                   fp_out;
  struct buf *             ib;
  struct buf *             ob;
  struct sd_markdown *     markdown;
  struct mdn_config  *     setting;
  struct sd_callbacks      callbacks;
  struct blender_renderopt options;

  /* Get current working directory */

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
        out  = optarg;
        mode = FILE_MODE;
        break;
      case ':':
        if (optopt == 'f') {
          sderror(&ret, "No file is given");
          return ret;
        }
        if (optopt == 'o') {
          out  = "/dev/stdout";
          mode = FILE_MODE;
        }
        break;
      default:
        break;
    }
  }
 if (isatty(STDIN_FILENO)) {
   if (argc < 2) {
     usage();
     return EXIT_FAILURE;
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
   title = get_file_ext(in, '/');
   if ((strcmp(title, "")) == 0)
     title = in;

   ext = get_file_ext(title, '.');
 }
 else {
   ext = "html";
   fp_in = stdin;
   ext = "md";
 }

  ib = bufnew(READ_UNIT);
  bufgrow(ib, READ_UNIT);
  while ((ret = fread(ib->data + ib->size, 1, ib->asize - ib->size, fp_in)) > 0) {
    ib->size += ret;
    bufgrow(ib, ib->size + READ_UNIT);
  }
  if (isatty(STDIN_FILENO))
    fclose(fp_in);

  setting = default_config_new();

  ob = bufnew(OUTPUT_UNIT);
  if ((strcmp(ext, "html")) == 0) {
    bufput(ob, ib->data, ib->size);
  }
  else {
    bufprintf(ob, "<html><head><title>%s(7)</title></head><body>", title);
    if (((strcmp(ext, "")) == 0) || ((strcmp(ext, "txt")) == 0)) {
      bufputs(ob, "<pre>");
      bufput(ob, ib->data, ib->size);
      bufputs(ob, "</pre>");
    }
    else if (((strcmp(ext, "md")) == 0) || (strcmp(ext, "MD")) == 0) {
      sdblender_renderer(&callbacks, &options, 0);
      markdown = sd_markdown_new(extensions, 16, &callbacks, &options);
      sd_markdown_render(ob, ib->data, ib->size, markdown);
      sd_markdown_free(markdown);
    }
    bufputs(ob, "</body></html>\n");
  }
  bufrelease(ib);

/* Prepare for parsing */
  if (mode) {  // * output to file
    if (!(fp_out = fopen(out, "w"))) {
      sderror(&ret, strerror(errno));
      goto clean_up;
    }
    else {
      fwrite((void *)ob->data, ob->size, 1, fp_out);
      fclose(fp_out);
    }
  }
  else if (!isatty(STDOUT_FILENO)) {
    fwrite((void *)ob->data, ob->size, 1, stdout);
  }

  else {  // * output to mandown pager
    setting = config(setting);
    ret = view(setting, ob, href);
  }

  /* Clean up */
clean_up:
  bufrelease(ob);
  default_config_free(setting);
  return ret;
}
