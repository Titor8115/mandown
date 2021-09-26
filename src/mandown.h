#ifndef MDN_MANDOWN_H
#define MDN_MANDOWN_H

#include "buffer.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PAGE_MODE 0
#define FILE_MODE 1

extern struct mdn_cfg *configure();
extern int view(const struct mdn_cfg *, const struct buf *, int);

#ifdef __cplusplus
}
#endif

#endif /* MDN_MANDOWN_H */