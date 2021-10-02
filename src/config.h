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
#include "buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define READ_UNIT 1024
#define OUTPUT_UNIT 64

#ifdef DEBUG
#define RC_PREFIX "/test/mdnrc.cfg"
#else
#define RC_PREFIX "/.config/mdn/mdnrc"
#endif

#define RC_VERSION "1.0.0"
#define SCHEME_MDN 0
#define SCHEME_LESS 1
#define SCHEME_VIM 2

struct scheme {
  int key;
  int command;
  struct buf *str_form;
};

struct mdn_cfg {
  struct scheme map[14];
  char  control_scheme[5];
  int   use_mouse;
  int   indent;
};

void sd_info(char *);
void sd_error(char *);
void sd_warn(char *);


#define sdinfo(string)  sd_info((char *)string)
#define sderror(string) sd_error((char *)string)
#define sdwarn(string)  sd_warn((char *)string)

struct mdn_cfg *default_rc_new();
void default_rc_free(struct mdn_cfg *);

struct mdn_cfg *configure();

#ifdef __cplusplus
}
#endif

#endif /* MDN_CONFIG_H */
