/**
 * Copyright (C) 2019 Tianze Han
 *
 * This file is part of Mandown.
 *
 * Mandown is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mandown is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Mandown.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "config.h"

static struct mdn_cfg rc_rule = {
    .use_mouse = CONFIG_TRUE,
    .indent = 7,
    .control_scheme = "less",
};

void sd_info(char *output) {
  if (!isatty(STDOUT_FILENO))
    return;

  fprintf(stdout, "%sNote: %s%s\n", "\033[36m", "\033[0m", output);
}

void sd_error(char *output) {
  if (!isatty(STDERR_FILENO))
    return;

  fprintf(stderr, "%sError: %s%s\n", "\033[31m", "\033[0m", output);
}

void sd_warn(char *output) {
  if (!isatty(STDERR_FILENO))
    return;

  fprintf(stderr, "%sWarn: %s%s\n", "\033[33m", "\033[0m", output);
}

struct mdn_cfg *get_user_rc(config_t *user, struct mdn_cfg *config, FILE *fp) {
  config_t update;
  config_setting_t *setting;

  config_init(&update);
  setting = config_lookup(user, "use_mouse");
  if (!setting) {
    if (fp != NULL) {
      // fputs("# If you want Terminal Emulator handle mouse event\n# turn off
      // \"use_mouse\"\n", fp); setting = config_setting_add(&update.root,
      // "use_mouse", CONFIG_TYPE_BOOL); config_setting_set_bool(setting,
      // rc_rule.use_mouse); config_write(&update, fp);
      // config_setting_remove(&update.root, "use_mouse");
      fprintf(fp,
              "# If you want Terminal Emulator handle mouse event\n"
              "# turn off \"use_mouse\"\n"
              "use_mouse = %s;\n\n",
              (rc_rule.use_mouse ? "true" : "false"));
    }
  } else
    rc_rule.use_mouse = config_setting_get_bool(setting);

  setting = config_lookup(user, "indent");
  if (!config_lookup_int(user, "indent", &(rc_rule.indent))) {
    if (fp != NULL) {
      fprintf(fp,
              "# Indent controls where manual's content start for each line\n"
              "indent = %d;\n\n",
              rc_rule.indent);
    }
  } else
    rc_rule.indent = config_setting_get_int(setting);

  setting = config_lookup(user, "control_scheme");
  if (!setting) {
    if (fp != NULL) {
      fprintf(fp,
              "# Supported Keybinding: \"mdn\", \"vim\", \"less\"\n"
              "control_scheme = \"%s\";\n\n",
              rc_rule.control_scheme);
    }
  } else {
    rc_rule.control_scheme[0] = config_setting_get_string(setting)[0];
  }

  return config;
}

struct mdn_cfg *configure() {
  config_t cfg;
  char *env;
  char *rc_path;
  FILE *fp_rc;

  /* Setup default path for rc file based on user */
#ifdef DEBUG
  env = getenv("PWD");
  rc_path = malloc(strlen(env) + strlen("/test/mdnrc") + 1);
  strcpy(rc_path, env);
  strcat(rc_path, "/test");
#else
  env = getenv("XDG_CONFIG_HOME");
  if (env) {
    rc_path = malloc(strlen(env) + strlen("/mdn/mdnrc") + 1);
    strcpy(rc_path, env);
    strcat(rc_path, "/mdn");
  } else {
    env = getenv("HOME");
    rc_path = malloc(strlen(env) + strlen("/.config/mdn/mdnrc") + 1);
    strcpy(rc_path, env);
    strcat(rc_path, "/.config/mdn");
  }
#endif

  config_init(&cfg);

  if (mkdir(rc_path, 0755) && errno != EEXIST) {
    sdwarn("Failed to access mdnrc, switching to builtin setting");
  }
  else {
    strcat(rc_path, "/mdnrc");
    fp_rc = fopen(rc_path, "a+");
    if (!fp_rc) {
      sdwarn("Failed to access mdnrc, switching to builtin setting");
    } else {
      config_read(&cfg, fp_rc);
      get_user_rc(&cfg, &rc_rule, fp_rc);
      fclose(fp_rc);
    }
  }
  
  /* Clean up */
  config_destroy(&cfg);
  free(rc_path);
  return &rc_rule;
}
