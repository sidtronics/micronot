#include "config.h"
#include "utils.h"
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

static char *_trim(char *s) {

  while (isspace(*s))
    s++;
  if (*s == 0)
    return s;

  char *end = s + strlen(s) - 1;
  while (end > s && isspace(*end))
    *end-- = 0;

  return s;
}

static void _parse_field_ul(const char *key, const char *val,
                            unsigned long *res) {
  if (!utils_parse_ul(val, res, 10))
    fprintf(stderr, "[unotd:config] ERROR: invalid value '%s' for key '%s'\n",
            val, key);
}

static void _parse_field_u16(const char *key, const char *val, u_int16_t *res) {
  if (!utils_parse_u16(val, res, 10))
    fprintf(stderr, "[unotd:config] ERROR: invalid value '%s' for key '%s'\n",
            val, key);
}

static void _parse_field_dbl(const char *key, const char *val, double *res) {
  if (!utils_parse_dbl(val, res))
    fprintf(stderr, "[unotd:config] ERROR: invalid value '%s' for key '%s'\n",
            val, key);
}

static void _parse_color(const char *key, char *str, unsigned long *res) {

  // color string: #RRGGBB

  if (!(strlen(str) == 7 && *str == '#')) {
    fprintf(stderr,
            "[unotd:config] ERROR: invalid color string '%s' for key '%s'\n",
            str, key);
    return;
  }

  if (!utils_parse_ul(str + 1, res, 16))
    fprintf(stderr, "[unotd:config] ERROR: invalid color '%s' for key '%s'\n",
            str + 1, key);
}

static void _parse_origin(const char *origin_str, Origin *res) {

  if (strcmp(origin_str, "top-right") == 0)
    *res = UNOT_ORIGIN_TOP_RIGHT;
  else if (strcmp(origin_str, "top-left") == 0)
    *res = UNOT_ORIGIN_TOP_LEFT;
  else if (strcmp(origin_str, "bottom-right") == 0)
    *res = UNOT_ORIGIN_BOTTOM_RIGHT;
  else if (strcmp(origin_str, "bottom-left") == 0)
    *res = UNOT_ORIGIN_BOTTOM_LEFT;
  else
    fprintf(stderr, "[unotd:config] ERROR: invalid origin '%s'\n", origin_str);
}

typedef enum _ConfigSection {
  CONFIG_SECTION_NONE,
  CONFIG_SECTION_UNKNOWN,
  CONFIG_SECTION_GLOBAL,
  CONFIG_SECTION_INDICATORS
} ConfigSection;

void config_load(Config *cfg, const char *filename) {

  cfg->indicators = NULL;
  cfg->indicators_count = 0;
  size_t indicators_capacity = 0;

  FILE *f = fopen(filename, "r");
  if (!f) {
    perror("fopen");
    exit(1);
  }

  ConfigSection section = CONFIG_SECTION_NONE;
  int line_num = 0;
  char line[256] = {0};
  while (fgets(line, sizeof(line), f)) {

    line_num++;
    char *start = _trim(line);
    if (*start == '\0' || *start == '#')
      continue;

    if (*start == '[') {
      char *end = strchr(start + 1, ']');
      if (!end) {
        fprintf(
            stderr,
            "[unotd:config] ERROR: missing ']' in config file at line no: %d\n",
            line_num);
        continue;
      }

      *end = '\0';

      if (strncmp(start + 1, "global", strlen("global")) == 0)
        section = CONFIG_SECTION_GLOBAL;
      else if (strncmp(start + 1, "indicators", strlen("indicators")) == 0)
        section = CONFIG_SECTION_INDICATORS;
      else {
        fprintf(stderr,
                "[unotd:config] ERROR: unknown section '%s' at line no: %d\n",
                start + 1, line_num);
        section = CONFIG_SECTION_UNKNOWN;
      }
      continue;
    }

    char *eq = strchr(start, '=');
    if (!eq) {
      fprintf(stderr, "[unotd:config] ERROR: missing '=' at line no: %d\n",
              line_num);
      continue;
    }

    *eq = 0;
    char *key = _trim(start);
    if (!*key) {
      fprintf(stderr, "[unotd:config] ERROR: missing key\n");
      continue;
    }

    char *value = _trim(eq + 1);
    if (!*value) {
      fprintf(stderr, "[unotd:config] ERROR: missing value at line no: %d\n",
              line_num);
      continue;
    }

    if (*value == '"') {
      value = value + 1;
      char *end = strchr(value, '"');
      if (!end) {
        fprintf(stderr, "[unotd:config] ERROR: missing '\"' at line no: %d\n",
                line_num);
        continue;
      }
      *end = 0;
    }

    switch (section) {

    case CONFIG_SECTION_GLOBAL:

      if (strcmp(key, "origin") == 0)
        _parse_origin(value, &cfg->origin);
      else if (strcmp(key, "x_padding") == 0)
        _parse_field_u16(key, value, &cfg->x_padding);
      else if (strcmp(key, "y_padding") == 0)
        _parse_field_u16(key, value, &cfg->y_padding);
      else if (strcmp(key, "x_offset") == 0)
        _parse_field_u16(key, value, &cfg->x_offset);
      else if (strcmp(key, "y_offset") == 0)
        _parse_field_u16(key, value, &cfg->y_offset);
      else if (strcmp(key, "spacing") == 0)
        _parse_field_u16(key, value, &cfg->spacing);
      else if (strcmp(key, "border_size") == 0)
        _parse_field_u16(key, value, &cfg->border_size);
      else if (strcmp(key, "gap_size") == 0)
        _parse_field_u16(key, value, &cfg->gap_size);
      else if (strcmp(key, "indicator_size") == 0)
        _parse_field_dbl(key, value, &cfg->indicator_size);
      else if (strcmp(key, "font") == 0)
        cfg->font = strdup(value);
      else if (strcmp(key, "timeout") == 0)
        _parse_field_ul(key, value, &cfg->timeout);
      else if (strcmp(key, "bg_color") == 0)
        _parse_color(key, value, &cfg->bg_color);
      else if (strcmp(key, "text_color") == 0)
        _parse_color(key, value, &cfg->text_color);
      else if (strcmp(key, "indicator_color") == 0)
        _parse_color(key, value, &cfg->indicator_color);
      else if (strcmp(key, "border_color") == 0)
        _parse_color(key, value, &cfg->border_color);
      else
        fprintf(stderr,
                "[unotd:config] ERROR: unknown field: '%s' at line no: %d\n",
                key, line_num);
      break;

    case CONFIG_SECTION_INDICATORS:

      if (cfg->indicators_count >= indicators_capacity) {

        indicators_capacity =
            indicators_capacity == 0 ? 10 : indicators_capacity * 2;

        cfg->indicators =
            realloc(cfg->indicators, indicators_capacity * sizeof(char *));

        ASSERT(cfg->indicators && "config_load: failed reallocating");
      }

      size_t name_len = strlen(key) + 1;
      size_t istr_len = strlen(value) + 1;
      size_t idx = cfg->indicators_count;
      cfg->indicators[idx] = malloc(name_len + istr_len);
      memcpy(cfg->indicators[idx], key, name_len);
      memcpy(cfg->indicators[idx] + name_len, value, istr_len);
      cfg->indicators_count++;
      break;

    case CONFIG_SECTION_NONE:
      fprintf(
          stderr,
          "[unotd:config] ERROR: entry without any section at line no: %d\n",
          line_num);
      break;

    case CONFIG_SECTION_UNKNOWN:
      break;
    }
  }

  if (cfg->indicators_count != 0)
    cfg->indicators =
        realloc(cfg->indicators, cfg->indicators_count * sizeof(char *));

  fclose(f);
}
