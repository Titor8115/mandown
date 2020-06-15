#ifndef MDN_MANDOWN_H
#define MDN_MANDOWN_H

#ifdef __cplusplus
extern "C" {
#endif

#define READ_UNIT 1024
#define OUTPUT_UNIT 64

#define PAGE_MODE 0
#define FILE_MODE 1

#define LINE_FOLD 7

typedef struct style Style;
struct style {
  int color;
  int firAttr;
  int secAttr;
};

typedef struct config Config;
struct config {
  char mode;
  int  lineFold;
  // FILE *fp_out;
  // todo: add more setting
};

#define sdinfo(string) sd_info((char *)string)
#define sderror(string)   sd_error((char *)string)
#define sdwarn(string) sd_warn((char *)string)

extern void sd_info(char *);
extern void sd_error(char *);
extern void sd_warn(char *);

#ifdef __cplusplus
}
#endif

#endif /* MDN_MANDOWN_H */