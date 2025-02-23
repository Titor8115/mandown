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

int render_buf(struct buf *ib, const char *ext, const char *title, const char *out)
{
  FILE *                   fp_out;
  struct buf *             ob;
  struct sd_markdown *     markdown;
  struct sd_callbacks      callbacks;
  struct blender_renderopt options;
  int                      ret   = EXIT_FAILURE;
  unsigned int             extensions = MKDEXT_FENCED_CODE | MKDEXT_NO_INTRA_EMPHASIS |
                                        MKDEXT_TABLES | MKDEXT_AUTOLINK |
                                        MKDEXT_STRIKETHROUGH;

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

  /* Prepare for parsing */
  if (out != NULL) {
    if (!(fp_out = fopen(out, "w"))) {
      sderror(strerror(errno));
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

  else {  /* output to mandown pager */
    ret = view(ob, href);
  }

  /* Clean up */
clean_up:
  bufrelease(ob);
  // default_rc_free(setting);
  return ret;
}

int render_file(FILE *fp, const char *ext, const char *title, const char *out)
{
  struct buf *ib = bufnew(READ_UNIT);
  int        ret;

  if (ib == NULL)
    return EXIT_FAILURE;
  bufgrow(ib, READ_UNIT);
  while ((ret = fread(ib->data + ib->size, 1, ib->asize - ib->size, fp)) > 0) {
    ib->size += ret;
    bufgrow(ib, ib->size + READ_UNIT);
  }
  return render_buf(ib, ext, title, out);
}

int render_str(const char *str, const char *ext, const char *title, const char *out)
{
  struct buf *ib = bufnew(READ_UNIT);

  if (ib == NULL)
    return EXIT_FAILURE;
  bufputs(ib, str);
  return render_buf(ib, ext, title, out);
}
