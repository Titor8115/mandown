#ifndef MDN_CONFIG_H
#define MDN_CONFIG_H

#include <libconfig.h>
#include "buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define READ_UNIT 1024
#define OUTPUT_UNIT 64

#define RC_PREFIX "/.config/mdn/mdnrc"

#define RC_VERSION "1.0.3"
#define SCHEME_MDN 0
#define SCHEME_LESS 1
#define SCHEME_VIM 2

struct mdn_cfg {
  char  control_scheme[5];
  int   use_mouse;
  int   indent;
};

void sd_info(char *);
void sd_error(char *);
void sd_warn(char *);

struct mdn_cfg *default_rc_new();
void default_rc_free(struct mdn_cfg *);

#define sdinfo(string)  sd_info((char *)string)
#define sderror(string) sd_error((char *)string)
#define sdwarn(string)  sd_warn((char *)string)

#ifdef __cplusplus
}
#endif

#endif /* MDN_CONFIG_H */