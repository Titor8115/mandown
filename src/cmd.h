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

#ifndef MDN_CMD_H
#define MDN_CMD_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  CMD_ERR = -1,
  CMD_OK,
  CMD_EXIT,
  CMD_RESIZE_EVENT,
  CMD_MOUSE_EVENT,

  CMD_SELECT_COMFIRM = 100,
  CMD_SELECT_HREF,
  CMD_GOTO_DOWN,
  CMD_GOTO_UP,
  CMD_GOTO_NPAGE,
  CMD_GOTO_PPAGE,
  CMD_GOTO_TOF,
  CMD_GOTO_EOF,

} mdn_command;

mdn_command less_cmd_scheme(int);
mdn_command mdn_cmd_scheme(int);
mdn_command vim_cmd_scheme(int);

#ifdef __cplusplus
}
#endif

#endif /* MDN_CMD_H */
