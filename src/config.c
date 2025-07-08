#include "config.h"
#include "utils.h"
#include <ctype.h>
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
    fprintf(stderr, "error: invalid value '%s' for key '%s'\n", val, key);
}

static void _parse_field_u16(const char *key, const char *val, u_int16_t *res) {
  if (!utils_parse_u16(val, res, 10))
    fprintf(stderr, "error: invalid value '%s' for key '%s'\n", val, key);
}

static void _parse_field_dbl(const char *key, const char *val, double *res) {
  if (!utils_parse_dbl(val, res))
    fprintf(stderr, "error: invalid value '%s' for key '%s'\n", val, key);
}

static void _parse_color(const char *key, char *str, unsigned long *res) {

  // color string: "#RRGGBB"

  if (!(strlen(str) == 9 && strncmp(str, "\"#", 2) == 0 && str[8] == '"')) {
    fprintf(stderr, "error: invalid color string '%s' for key '%s'\n", str,
            key);
    return;
  }

  str[8] = 0;

  if (!utils_parse_ul(str + 2, res, 16))
    fprintf(stderr, "error: invalid color '%s' for key '%s'\n", str + 2, key);
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
    fprintf(stderr, "error: invalid origin '%s'\n", origin_str);
}

void config_load(Config *cfg, const char *filename) {

  FILE *f = fopen(filename, "r");
  if (!f) {
    perror("fopen");
    exit(1);
  }

  char line[256] = {0};
  while (fgets(line, sizeof(line), f)) {
    char *s = _trim(line);
    if (*s == '\0' || *s == '#')
      continue;

    char *eq = strchr(s, '=');
    if (!eq)
      continue;

    *eq = 0;
    char *key = _trim(s);
    char *value = _trim(eq + 1);

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
      fprintf(stderr, "error: unknown field: '%s'\n", key);
  }

  fclose(f);
}
