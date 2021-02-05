#include "dom.h"

struct dom_link *
dom_link_new(struct buf *buf)
{
  struct dom_link *ret;
  ret = malloc(sizeof(struct dom_link));

  if (ret) {
    ret->buf  = buf;
    ret->attr = 0;
    ret->next = NULL;
    ret->fold = 0;
    ret->prop = 0;
  }
  return ret;
}

void dom_link_append(struct dom_link **head, struct dom_link *link)
{
  struct dom_link *p = *head;
  if (link == NULL)
    return;

  if (*head == NULL) {
    *head = link;
    return;
  }

  while (p->next != NULL)
    p = p->next;
  p->next = link;
}

void dom_link_free(struct dom_link *head)
{
  struct dom_link *link;

  while (head != NULL) {
    link = head;
    head = head->next;
    bufrelease(link->buf);
    free(link);
  }
}

void dom_link_reset(struct dom_link *link)
{
  if (!link)
    return;
  bufrelease(link->buf);
  dom_link_free(link->next);
  link->buf  = NULL;
  link->attr = 0;
  link->next = NULL;
  link->fold = 0;
  link->prop = 0;
}

struct dom_href_stack *
dom_href_new(struct buf *buf)
{
  struct dom_href_stack *ret;
  ret = malloc(sizeof(struct dom_href_stack));

  if (ret) {
    ret->url   = buf;
    ret->index = 0;
    ret->beg_y = 0;
    ret->beg_x = 0;
  }
  return ret;
}

void dom_href_free(struct dom_href_stack *href)
{
  if (!href)
    return;

  bufrelease(href->url);
  free(href);
}

int dom_stack_new(struct stack *st, size_t size)
{
  return stack_init(st, size);
}

int dom_stack_push(struct stack *st, struct dom_href_stack *item)
{
  if (!st || !item)
    return 0;

  return stack_push(st, (void *)item);
}

struct dom_href_stack *
dom_stack_pop(struct stack *st)
{
  return stack_pop(st);
}

struct dom_href_stack *
dom_stack_top(struct stack *st)
{
  return stack_top(st);
}

struct dom_href_stack *
dom_stack_bot(struct stack *st)
{
  return stack_bot(st);
}

struct dom_href_stack *
dom_stack_find(struct stack *st, size_t start, int mincol, int maxcol, int x)
{
  size_t                 i;
  struct dom_href_stack *tmp;

  if (!st->size)
    return NULL;

  for (i = start; i < st->size; i++) {
    tmp = st->item[i];
    if ((mincol <= tmp->beg_y) && (tmp->beg_y <= maxcol) && (tmp != st->item[start])) {
      if (x == tmp->beg_x)
        return tmp;
    }
  }

  for (i = 0; i < st->size; i++) {
    tmp = st->item[i];
    if ((mincol <= tmp->beg_y) && (tmp->beg_y <= maxcol)) {
      return tmp;
    }
  }

  return st->item[start];
}

void dom_stack_free(struct stack *st)
{
  if (!st)
    return;

  while (st->size != 0) {
    dom_href_free(dom_stack_pop(st));
  }
  stack_free(st);
}
