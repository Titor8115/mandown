#include "config.h"

#include <string.h>
#include <unistd.h>

struct mdn_config *
default_config_new()
{
  struct mdn_config *ret;
  ret = malloc(sizeof(struct mdn_config));

  if (ret) {
    ret->indent = 7;
  }
  return ret;
};

void default_config_free(struct mdn_config *config)
{
  if (!config)
    return;
  free(config);
}
struct mdn_config *
user_config(config_t *user, struct mdn_config *def)
{
  int indent;
  if (config_lookup_int(user, "indent", &indent))
    def->indent = indent;

  return def;
}

static void generate_rc(struct mdn_config *ib, char *path)
{
  config_t cfg;
  config_setting_t *root, *setting;

  config_init(&cfg);
  root = config_root_setting(&cfg);

  /* Pager configuration group */
  setting = config_setting_add(root, "indent", CONFIG_TYPE_INT);
  config_setting_set_int(setting, ib->indent);

  if (!config_write_file(&cfg, path)) {
    fprintf(stdout, "Cant write Config File\n");
  }
  config_destroy(&cfg);
}

struct mdn_config *
config(struct mdn_config *configure)
{
  config_t cfg;
  const char *rc_path = "/.config/mdn/mdnrc";
  char *file_path;

  file_path = malloc(strlen(getenv("HOME")) + strlen(rc_path) + 1);
  strcpy(file_path, getenv("HOME"));
  strcat(file_path, rc_path);
  config_init(&cfg);

  if (!(config_read_file(&cfg, file_path))) {
    generate_rc(configure, file_path);
    config_read_file(&cfg, file_path);
  }
  configure = user_config(&cfg, configure);
  config_destroy(&cfg);
  return configure;
}