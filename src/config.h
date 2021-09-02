#ifndef MDN_CONFIG_H
#define MDN_CONFIG_H

#include <libconfig.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

struct mdn_config {
  int indent;
};

struct mdn_config* default_config_new();
void               default_config_free(struct mdn_config*);
struct mdn_config* config(struct mdn_config*);

#ifdef __cplusplus
}
#endif

#endif /* MDN_CONFIG_H */