#ifndef MDN_DOM_H
#define MDN_DOM_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "buffer.h"

#define MANDOWN_HTML_SHIFT 8
#define MANDOWN_BITS(mask, shift) ((unsigned int)(mask)) << (shift)

#define P_DEFAULT       (1 - 1)
#define P_CONTROL       MANDOWN_BITS(1, 0)
#define P_ATTR_ON       MANDOWN_BITS(1, 1)
#define P_LINE_BREAK    MANDOWN_BITS(1, 2)
#define P_SELF_FORMAT   MANDOWN_BITS(1, 3)
#define P_HYPER_LINK    MANDOWN_BITS(1, 4)

struct dom_link_t {
  struct dom_link_t *next;
  struct buf *       buf;
  unsigned long      buf_attr;
  int                fold;
  unsigned int       prop;
};

struct dom_link_t * dom_link_new(struct buf *);

void dom_link_append(struct dom_link_t **, struct dom_link_t *);

void dom_link_free(struct dom_link_t *);

void dom_link_reset(struct dom_link_t *);



#endif /* MDN_DOM_H */
