#ifndef MDN_MANDOWN_H
#define MDN_MANDOWN_H

#include "buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define READ_UNIT 1024
#define OUTPUT_UNIT 64

#define PAGE_MODE 0
#define FILE_MODE 1

#define USE_COLOR 1
#define LINE_FOLD 7

typedef struct config Config;
struct config {
  char         mode;
  int          fold;
  // FILE *fp_out;
  // todo: add more setting
};

#define sdinfo(string)  sd_info((char *)string)
#define sderror(ret, string) sd_error(ret, (char *)string)
#define sdwarn(string)  sd_warn((char *)string)

extern void sd_info(char *);
extern void sd_error(int *, char *);
extern void sd_warn(char *);

extern int view(const Config *, const struct buf *, int);

#ifdef __cplusplus
}
#endif

#endif /* MDN_MANDOWN_H */