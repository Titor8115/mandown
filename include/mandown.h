#ifndef MDN_MANDOWN_H
#define MDN_MANDOWN_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif
#define READ_UNIT 1024
#define OUTPUT_UNIT 64

#define PAGE_MODE 0
#define FILE_MODE 1

#define LINE_FOLD 7

typedef struct config Config;
struct config {
  char  mode;
  int  line_fold;
  FILE *fp_out;
  // todo: add more setting
};

#define sdmessage(string) sd_message((char *)string)
#define sderror(string) sd_error((char *)string)
#define sdwarning(string) sd_warning((char *)string)

extern void sd_message(char *);
extern void sd_error(char *);
extern void sd_warning(char *);

#ifdef __cplusplus
}
#endif

#endif /* MDN_MANDOWN_H */