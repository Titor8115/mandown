#include "dom.h"

struct dom_link_t *
dom_link_new(struct buf *buf)
{
  struct dom_link_t *ret;
  ret = malloc(sizeof(struct dom_link_t));

  if (ret) {
    ret->buf      = buf;
    ret->buf_attr = 0;
    ret->next     = NULL;
    ret->fold     = 0;
    ret->prop     = 0;
  }
  return ret;
}

void dom_link_append(struct dom_link_t **head, struct dom_link_t *link)
{
  struct dom_link_t *p = *head;
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

void dom_link_free(struct dom_link_t *head)
{
  struct dom_link_t *link;

  while (head != NULL) {
    link = head;
    head = head->next;
    bufrelease(link->buf);
    free(link);
  }
}

void dom_link_reset(struct dom_link_t *link)
{
  if (!link)
    return;
  bufrelease(link->buf);
  dom_link_free(link->next);
  link->buf      = NULL;
  link->buf_attr = 0;
  link->next     = NULL;
  link->fold     = 0;
  link->prop     = 0;
}
