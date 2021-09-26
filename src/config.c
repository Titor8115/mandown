#include "config.h"

#include <libconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static struct mdn_cfg default_cfg =
{
        .use_mouse = CONFIG_TRUE,
        .indent = 7,
        .control_scheme = "less"
};

void sd_info(char *output)
{
  if (!isatty(STDOUT_FILENO))
    return;

  fprintf(stdout, "%sNote: %s%s\n", "\033[36m", "\033[0m", output);
}

void sd_error(char *output)
{
  if (!isatty(STDERR_FILENO))
    return;

  fprintf(stderr, "%sError: %s%s\n", "\033[31m", "\033[0m", output);
}

void sd_warn(char *output)
{
  if (!isatty(STDERR_FILENO))
    return;

  fprintf(stderr, "%sWarn: %s%s\n", "\033[33m", "\033[0m", output);
}

struct mdn_cfg *
get_user_rc(config_t *user, struct mdn_cfg *config, FILE *fp)
{
  const char *control_scheme;
  config_setting_t *setting;

  setting = config_lookup(user, "use_mouse");
  if (!setting) {
    if (fp != NULL) {
      fprintf(fp,
              "# If you want Terminal Emulator handle mouse event"
              "# turn off \"use_mouse\"\n"
              "use_mouse = %s;\n\n",
              (default_cfg.use_mouse ? "true" : "false"));
    }
  }
  else
    default_cfg.use_mouse = config_setting_get_bool(setting);

  setting = config_lookup(user, "indent");
  if (!config_lookup_int(user, "indent", &(default_cfg.indent))) {
    if (fp != NULL) {
      fprintf(fp,
              "# Indent controls where manual's content start for each line\n"
              "indent = %d;\n\n",
              default_cfg.indent);
    }
  }
  else
    default_cfg.indent = config_setting_get_int(setting);

  setting = config_lookup(user, "control_scheme");
  if (!setting) {
    if (fp != NULL) {
      fprintf(fp,
              "# Supported Keybinding: \"mdn\", \"vim\", \"less\"\n"
              "control_scheme = \"%s\";\n\n",
              default_cfg.control_scheme);
    }
  }
  else {
    control_scheme = config_setting_get_string(setting);
    default_cfg.control_scheme[0] = config_setting_get_string(setting)[0];
  }

  return config;
}

struct mdn_cfg *
configure()
{
  config_t cfg;
  char *rc_path;
  FILE *fp_rc;

  /* Setup default path for rc file based on user */
  rc_path = malloc(strlen(getenv("HOME")) + strlen(RC_PREFIX) + 1);
  strcpy(rc_path, getenv("HOME"));
  strcat(rc_path, RC_PREFIX);

  config_init(&cfg);
  fp_rc = fopen(rc_path, "a+");
  if (!fp_rc) {
    sdwarn("Failed to Update: ~/.config/mdn/mdnrc\nSwitch to builtin setting");
  }
  else {
    config_read(&cfg, fp_rc);
    get_user_rc(&cfg, &default_cfg, fp_rc);
  }
  // }

  /* Clean up */
  fclose(fp_rc);
  config_destroy(&cfg);
  free(rc_path);
  return &default_cfg;
}