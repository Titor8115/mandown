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
#define P_SELF_FORM MANDOWN_BITS(1, 3)
#define P_HYPERLINK MANDOWN_BITS(1, 4)

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

struct dom_href_stack *dom_stack_find(struct stack *, size_t, int, int, int);

void dom_stack_free(struct stack *);

#ifdef __cplusplus
}
#endif

#endif /* MDN_DOM_H */
