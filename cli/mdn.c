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

#include "mandown.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "blender.h"
#include "buffer.h"
#include "config.h"
#include "markdown.h"
#include "view.h"

void usage()
{
  fprintf(stdout, "%s\n", "mdn - Markdown Manual, a man(1) like markdown pager");
  fprintf(stdout, "%s\n\n", "Usage: mdn <title> [options...]");

  fprintf(stdout, "%-20s%s\n", "  -f, --file", "optional flag for filepath");
  fprintf(stdout, "%-20s%s\n", "  -h, --help", "this help text");
  fprintf(stdout, "%-20s%s\n\n", "  -o, --outpath", "xhtml version of input file");

  fprintf(stdout, "%s\n\n", "Custom setting: ~/.config/mdn/mdnrc");

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

  if (!dot || dot == file)
    return "";

  return dot + 1;
}

int main(int argc, char **argv)
{
  FILE *                   fp_in;
  const char *             in    = NULL;
  const char *             out   = NULL;
  const char *             ext   = "";
  const char *             title = "";
  int                      opt;
  int                      ret   = EXIT_FAILURE;

  while ((opt = getopt(argc, argv, ":f:ho:")) != -1) {
    switch (opt) {
      case 'f':
        in = optarg;
        break;
      case 'h':
        usage();
        return EXIT_SUCCESS;
      case 'o':
        out  = optarg;
        break;
      case ':':
        if (optopt == 'f') {
          sderror("No file is given");
          return EXIT_FAILURE;
        }
        if (optopt == 'o') {
          out  = "/dev/stdout";
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
      sderror("No file is given");
      return EXIT_FAILURE;
    }
    /* Reading file */
    fp_in = fopen(in, "r");
    if (!fp_in) {
      sderror(strerror(errno));
      return EXIT_FAILURE;
    }
    title = get_file_ext(in, '/');
    if ((strcmp(title, "")) == 0)
      title = in;

    ext = get_file_ext(title, '.');
  }
  else {
    fp_in = stdin;
    ext = "md";
  }

  render_file(fp_in, ext, title, out);
  if (isatty(STDIN_FILENO))
    fclose(fp_in);

  // default_rc_free(setting);
  return ret;
}
