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

#ifndef MDN_DOM_H
#define MDN_DOM_H

#include <stdint.h>
#include <stddef.h>

#include "buffer.h"
#include "stack.h"
#include "st_curses.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MANDOWN_BITS(mask, shift) ((uint8_t)(mask)) << (shift)

#define P_REGULAR   (1 - 1)
#define P_CONTROL   MANDOWN_BITS(1, 0)
#define P_END_A     MANDOWN_BITS(0, 1)
#define P_BEG_A     MANDOWN_BITS(1, 1)
#define P_SPLIT     MANDOWN_BITS(1, 2)
#define P_PREFIXED  MANDOWN_BITS(1, 3)
#define P_SECTION   MANDOWN_BITS(1, 4)
#define P_HYPERLINK MANDOWN_BITS(1, 5)

struct dom_link {
  struct dom_link *next;
  struct buf      *buf;
  attr_t          attr;
  int             indent;
  uint8_t         prop;
};

struct dom_href_stack {
  struct buf *url;
  size_t      index;
  int         beg_y;
  int         beg_x;
  int         end_y;
  int         end_x;
};

struct dom_link *dom_link_new(struct buf *);

void dom_link_append(struct dom_link **, struct dom_link *);

void dom_link_free(struct dom_link *);

void dom_link_reset(struct dom_link *);

struct dom_href_stack *dom_href_new(struct buf *);

// void dom_href_makepad(struct dom_href_stack *, int, int)

void dom_href_free(struct dom_href_stack *);

int dom_stack_new(struct stack *, size_t);

int dom_stack_push(struct stack *, struct dom_href_stack *);

struct dom_href_stack *dom_stack_pop(struct stack *);

struct dom_href_stack *dom_stack_top(struct stack *);

struct dom_href_stack *dom_stack_bot(struct stack *);

struct dom_href_stack *dom_stack_find(struct stack *, size_t, int, int, int, int);

void dom_stack_free(struct stack *);

#ifdef __cplusplus
}
#endif

#endif /* MDN_DOM_H */
