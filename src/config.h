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

#ifndef MDN_CONFIG_H
#define MDN_CONFIG_H

#include <libconfig.h>

#ifdef __cplusplus
extern "C" {
#endif

#define READ_UNIT   1024
#define OUTPUT_UNIT 64

#define RC_VERSION  "1.0.0"
#define SCHEME_MDN  0
#define SCHEME_LESS 1
#define SCHEME_VIM  2

#define HEADING_LEVELS 6

/* Color names accepted in mdnrc, the order must match the ncurses COLOR_*
 * constants so that the index can be used directly as a color pair id */
enum mdn_color {
  MDN_COLOR_DEFAULT = 0,
  MDN_COLOR_BLACK,
  MDN_COLOR_RED,
  MDN_COLOR_GREEN,
  MDN_COLOR_YELLOW,
  MDN_COLOR_BLUE,
  MDN_COLOR_MAGENTA,
  MDN_COLOR_CYAN,
  MDN_COLOR_WHITE,

  MDN_COLOR_MAX,
};

/* Style shared by every configurable element */
struct mdn_style {
  int  color;
  int  background;
  int  bold;
  int  italic;
  int  underline;
  int  dim;
};

struct mdn_cfg {
  char control_scheme[5];
  int  use_mouse;
  int  indent;

  /* Heading style, indent is per level starting at h1 */
  struct mdn_style heading;
  int              heading_indent[HEADING_LEVELS];

  /* Inline code and code block style, indent is extra indent of code blocks */
  struct mdn_style code;
  int              code_indent;

  /* Hyperlink style */
  struct mdn_style url;

  /* Blockquote style, prefix is printed in front of quoted content */
  struct mdn_style quote;
  char             quote_prefix[8];
  int              quote_indent;
};

void
sd_info(char *);
void
sd_error(char *);
void
sd_warn(char *);

#define sdinfo(string)  sd_info((char *)string)
#define sderror(string) sd_error((char *)string)
#define sdwarn(string)  sd_warn((char *)string)

struct mdn_cfg *
default_rc_new();
void
default_rc_free(struct mdn_cfg *);

struct mdn_cfg *
configure();

#ifdef __cplusplus
}
#endif

#endif /* MDN_CONFIG_H */
