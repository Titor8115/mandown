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

#include "config.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* Color names accepted in mdnrc, index is the value stored in mdn_style */
static const char *const color_names[MDN_COLOR_MAX] = {
    [MDN_COLOR_DEFAULT] = "default",
    [MDN_COLOR_BLACK]   = "black",
    [MDN_COLOR_RED]     = "red",
    [MDN_COLOR_GREEN]   = "green",
    [MDN_COLOR_YELLOW]  = "yellow",
    [MDN_COLOR_BLUE]    = "blue",
    [MDN_COLOR_MAGENTA] = "magenta",
    [MDN_COLOR_CYAN]    = "cyan",
    [MDN_COLOR_WHITE]   = "white",
};

static struct mdn_cfg rc_rule = {
    .use_mouse      = CONFIG_TRUE,
    .indent         = 7,
    .control_scheme = "less",

    .heading = {
        .color      = MDN_COLOR_DEFAULT,
        .background = MDN_COLOR_DEFAULT,
        .bold       = CONFIG_TRUE,
        .italic     = CONFIG_FALSE,
        .underline  = CONFIG_FALSE,
        .dim        = CONFIG_FALSE,
    },
    .heading_indent = {1, 0, 3, 4, 5, 6},

    .code = {
        .color      = MDN_COLOR_YELLOW,
        .background = MDN_COLOR_DEFAULT,
        .bold       = CONFIG_FALSE,
        .italic     = CONFIG_FALSE,
        .underline  = CONFIG_FALSE,
        .dim        = CONFIG_FALSE,
    },
    .code_indent = 2,

    .url = {
        .color      = MDN_COLOR_BLUE,
        .background = MDN_COLOR_DEFAULT,
        .bold       = CONFIG_FALSE,
        .italic     = CONFIG_FALSE,
        .underline  = CONFIG_TRUE,
        .dim        = CONFIG_FALSE,
    },

    .quote = {
        .color      = MDN_COLOR_DEFAULT,
        .background = MDN_COLOR_DEFAULT,
        .bold       = CONFIG_FALSE,
        .italic     = CONFIG_FALSE,
        .underline  = CONFIG_FALSE,
        .dim        = CONFIG_TRUE,
    },
    .quote_prefix = "│",
    .quote_indent = 0,
};

void
sd_info(char *output)
{
  if (!isatty(STDOUT_FILENO)) return;

  fprintf(stdout, "%sNote: %s%s\n", "\033[36m", "\033[0m", output);
}

void
sd_error(char *output)
{
  if (!isatty(STDERR_FILENO)) return;

  fprintf(stderr, "%sError: %s%s\n", "\033[31m", "\033[0m", output);
}

void
sd_warn(char *output)
{
  if (!isatty(STDERR_FILENO)) return;

  fprintf(stderr, "%sWarn: %s%s\n", "\033[33m", "\033[0m", output);
}

/* Look up a color name, returns -1 if the name is unknown */
static int
color_lookup(const char *name)
{
  if (!name) return -1;

  for (int i = MDN_COLOR_DEFAULT; i < MDN_COLOR_MAX; i++)
    if (!strcmp(name, color_names[i])) return i;

  return -1;
}

/* Read a color member of a style group, keep the current value if the member
 * is missing or invalid */
static void
style_read_color(config_setting_t *group, const char *name, int *dest)
{
  const char *name_str;
  int         color;

  if (!config_setting_lookup_string(group, name, &name_str)) return;

  color = color_lookup(name_str);
  if (color < 0)
    sdwarn("Unknown color in mdnrc, using builtin setting");
  else
    *dest = color;
}

/* Read a boolean member of a style group, keep the current value if the member
 * is missing or invalid */
static void
style_read_bool(config_setting_t *group, const char *name, int *dest)
{
  int value;

  if (config_setting_lookup_bool(group, name, &value)) *dest = value;
}

/* Read a style group (color, background, bold, italic, underline, dim) */
static void
style_read(config_setting_t *group, struct mdn_style *style)
{
  if (!group || !config_setting_is_group(group)) return;

  style_read_color(group, "color", &style->color);
  style_read_color(group, "background", &style->background);
  style_read_bool(group, "bold", &style->bold);
  style_read_bool(group, "italic", &style->italic);
  style_read_bool(group, "underline", &style->underline);
  style_read_bool(group, "dim", &style->dim);
}

/* Read the indent member of a style group, it accepts either a single value
 * applied to every level or a list with one value per level */
static void
style_read_indent(config_setting_t *group, int *dest, int levels)
{
  config_setting_t *setting;
  int               value;

  if (!group || !config_setting_is_group(group)) return;

  setting = config_setting_get_member(group, "indent");
  if (!setting) return;

  if (config_setting_is_number(setting)) {
    value = config_setting_get_int(setting);
    for (int i = 0; i < levels; i++) dest[i] = value;
  }
  else if (config_setting_is_aggregate(setting)) {
    for (int i = 0; i < levels && i < config_setting_length(setting); i++)
      dest[i] = config_setting_get_int_elem(setting, i);
  }
  else {
    sdwarn("Invalid indent in mdnrc, using builtin setting");
  }
}

/* Read the prefix member of a style group */
static void
style_read_prefix(config_setting_t *group, char *dest, size_t size)
{
  const char *prefix;

  if (!group || !config_setting_is_group(group)) return;
  if (!config_setting_lookup_string(group, "prefix", &prefix)) return;

  strncpy(dest, prefix, size - 1);
  dest[size - 1] = '\0';
}

/* Write the default value of a style group to the rc file */
static void
style_write(FILE *fp, const char *name, const struct mdn_style *style)
{
  fprintf(fp,
          "%s = {\n"
          "  # Colors: default, black, red, green, yellow, blue, magenta,\n"
          "  # cyan, white\n"
          "  color = \"%s\";\n"
          "  background = \"%s\";\n"
          "  bold = %s;\n"
          "  italic = %s;\n"
          "  underline = %s;\n"
          "  dim = %s;\n"
          "};\n\n",
          name, color_names[style->color], color_names[style->background],
          style->bold ? "true" : "false", style->italic ? "true" : "false",
          style->underline ? "true" : "false", style->dim ? "true" : "false");
}

/* Parse the rc file into rc_rule, missing settings are appended to the rc file
 * so that the user can discover and edit them */
struct mdn_cfg *
get_user_rc(config_t *user, struct mdn_cfg *config, FILE *fp)
{
  config_setting_t *setting;
  const char       *scheme;
  int               value;

  setting = config_lookup(user, "use_mouse");
  if (!setting) {
    if (fp != NULL) {
      fprintf(fp,
              "# If you want Terminal Emulator handle mouse event\n"
              "# turn off \"use_mouse\"\n"
              "use_mouse = %s;\n\n",
              (rc_rule.use_mouse ? "true" : "false"));
    }
  }
  else {
    rc_rule.use_mouse = config_setting_get_bool(setting);
  }

  setting = config_lookup(user, "indent");
  if (!setting) {
    if (fp != NULL) {
      fprintf(fp,
              "# Indent controls where manual's content start for each line\n"
              "indent = %d;\n\n",
              rc_rule.indent);
    }
  }
  else {
    rc_rule.indent = config_setting_get_int(setting);
  }

  setting = config_lookup(user, "control_scheme");
  if (!setting) {
    if (fp != NULL) {
      fprintf(fp,
              "# Supported Keybinding: \"mdn\", \"vim\", \"less\"\n"
              "control_scheme = \"%s\";\n\n",
              rc_rule.control_scheme);
    }
  }
  else {
    scheme = config_setting_get_string(setting);
    if (scheme) {
      strncpy(rc_rule.control_scheme, scheme,
              sizeof(rc_rule.control_scheme) - 1);
      rc_rule.control_scheme[sizeof(rc_rule.control_scheme) - 1] = '\0';
    }
  }

  setting = config_lookup(user, "heading");
  if (!setting) {
    if (fp != NULL) {
      fprintf(fp,
              "# Heading style (h1 - h6). \"indent\" accepts a single value "
              "for\n"
              "# every level or a list of six values, one per level starting "
              "at h1\n"
              "heading = {\n"
              "  indent = (%d, %d, %d, %d, %d, %d);\n"
              "  # Colors: default, black, red, green, yellow, blue, "
              "magenta,\n"
              "  # cyan, white\n"
              "  color = \"%s\";\n"
              "  background = \"%s\";\n"
              "  bold = %s;\n"
              "  italic = %s;\n"
              "  underline = %s;\n"
              "  dim = %s;\n"
              "};\n\n",
              rc_rule.heading_indent[0], rc_rule.heading_indent[1],
              rc_rule.heading_indent[2], rc_rule.heading_indent[3],
              rc_rule.heading_indent[4], rc_rule.heading_indent[5],
              color_names[rc_rule.heading.color],
              color_names[rc_rule.heading.background],
              rc_rule.heading.bold ? "true" : "false",
              rc_rule.heading.italic ? "true" : "false",
              rc_rule.heading.underline ? "true" : "false",
              rc_rule.heading.dim ? "true" : "false");
    }
  }
  else {
    style_read_indent(setting, rc_rule.heading_indent, HEADING_LEVELS);
    style_read(setting, &rc_rule.heading);
  }

  setting = config_lookup(user, "code");
  if (!setting) {
    if (fp != NULL) {
      fprintf(fp,
              "# Inline code and code block style\n"
              "code = {\n"
              "  # Extra indent applied to code blocks\n"
              "  indent = %d;\n"
              "  # Colors: default, black, red, green, yellow, blue, "
              "magenta,\n"
              "  # cyan, white\n"
              "  color = \"%s\";\n"
              "  background = \"%s\";\n"
              "  bold = %s;\n"
              "  italic = %s;\n"
              "  underline = %s;\n"
              "  dim = %s;\n"
              "};\n\n",
              rc_rule.code_indent, color_names[rc_rule.code.color],
              color_names[rc_rule.code.background],
              rc_rule.code.bold ? "true" : "false",
              rc_rule.code.italic ? "true" : "false",
              rc_rule.code.underline ? "true" : "false",
              rc_rule.code.dim ? "true" : "false");
    }
  }
  else {
    style_read_indent(setting, &rc_rule.code_indent, 1);
    style_read(setting, &rc_rule.code);
  }

  setting = config_lookup(user, "url");
  if (!setting) {
    if (fp != NULL) {
      fprintf(fp, "# Hyperlink style\n");
      style_write(fp, "url", &rc_rule.url);
    }
  }
  else {
    style_read(setting, &rc_rule.url);
  }

  setting = config_lookup(user, "quote");
  if (!setting) {
    if (fp != NULL) {
      fprintf(fp,
              "# Blockquote style\n"
              "quote = {\n"
              "  # Character printed in front of quoted content\n"
              "  prefix = \"%s\";\n"
              "  indent = %d;\n"
              "  # Colors: default, black, red, green, yellow, blue, "
              "magenta,\n"
              "  # cyan, white\n"
              "  color = \"%s\";\n"
              "  background = \"%s\";\n"
              "  bold = %s;\n"
              "  italic = %s;\n"
              "  underline = %s;\n"
              "  dim = %s;\n"
              "};\n",
              rc_rule.quote_prefix, rc_rule.quote_indent,
              color_names[rc_rule.quote.color],
              color_names[rc_rule.quote.background],
              rc_rule.quote.bold ? "true" : "false",
              rc_rule.quote.italic ? "true" : "false",
              rc_rule.quote.underline ? "true" : "false",
              rc_rule.quote.dim ? "true" : "false");
    }
  }
  else {
    style_read_prefix(setting, rc_rule.quote_prefix,
                      sizeof(rc_rule.quote_prefix));
    style_read_indent(setting, &rc_rule.quote_indent, 1);
    style_read(setting, &rc_rule.quote);
  }

  /* Silence unused variable warning when DEBUG is not defined */
  (void)value;

  return config;
}

struct mdn_cfg *
configure()
{
  config_t cfg;
  char    *env;
  char    *rc_path;
  FILE    *fp_rc;

  /* Setup default path for rc file based on user */
#ifdef DEBUG
  env     = getenv("PWD");
  rc_path = malloc(strlen(env) + strlen("/test/mdnrc") + 1);
  strcpy(rc_path, env);
  strcat(rc_path, "/test");
#else
  env = getenv("XDG_CONFIG_HOME");
  if (env) {
    rc_path = malloc(strlen(env) + strlen("/mdn/mdnrc") + 1);
    strcpy(rc_path, env);
    strcat(rc_path, "/mdn");
  }
  else {
    env     = getenv("HOME");
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
    }
    else {
      if (config_read(&cfg, fp_rc) == CONFIG_FALSE) {
        sdwarn("Failed to parse mdnrc, switching to builtin setting");
        config_destroy(&cfg);
        config_init(&cfg);
      }
      get_user_rc(&cfg, &rc_rule, fp_rc);
      fclose(fp_rc);
    }
  }

  /* Clean up */
  config_destroy(&cfg);
  free(rc_path);
  return &rc_rule;
}
