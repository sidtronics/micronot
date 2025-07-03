#include "config.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static unsigned long _parse_color(const char *hex) {

  if (hex[0] == '#')
    hex++;

  return strtoul(hex, NULL, 16);
}

static Origin _parse_origin(const char *s) {

  if (strcmp(s, "top-right") == 0)
    return UNOT_ORIGIN_TOP_RIGHT;
  if (strcmp(s, "top-left") == 0)
    return UNOT_ORIGIN_TOP_LEFT;
  if (strcmp(s, "bottom-right") == 0)
    return UNOT_ORIGIN_BOTTOM_RIGHT;
  if (strcmp(s, "bottom-left") == 0)
    return UNOT_ORIGIN_BOTTOM_LEFT;
  else
    fprintf(stderr, "error: invalid origin\n");

  return UNOT_ORIGIN_BOTTOM_RIGHT;
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
    if (*s == '\0' || *s == '#') continue;

    char *eq = strchr(s, '=');
    if (!eq) continue;

    *eq = 0;
    char *key = _trim(s);
    char *value = _trim(eq + 1);

    if (strcmp(key, "origin") == 0)
      cfg->origin = _parse_origin(value);
    else if (strcmp(key, "x_padding") == 0)
      cfg->x_padding = atoi(value);
    else if (strcmp(key, "y_padding") == 0)
      cfg->y_padding = atoi(value);
    else if (strcmp(key, "x_offset") == 0)
      cfg->x_offset = atoi(value);
    else if (strcmp(key, "y_offset") == 0)
      cfg->y_offset = atoi(value);
    else if (strcmp(key, "spacing") == 0)
      cfg->spacing = atoi(value);
    else if (strcmp(key, "border_size") == 0)
      cfg->border_size = atoi(value);
    else if (strcmp(key, "gap_size") == 0)
      cfg->gap_size = atoi(value);
    else if (strcmp(key, "indicator_size") == 0)
      cfg->indicator_size = atof(value);
    else if (strcmp(key, "timeout") == 0)
      cfg->timeout = strtoul(value, NULL, 10);
    else if (strcmp(key, "bg_color") == 0)
      cfg->bg_color = _parse_color(value);
    else if (strcmp(key, "text_color") == 0)
      cfg->text_color = _parse_color(value);
    else if (strcmp(key, "indicator_color") == 0)
      cfg->indicator_color = _parse_color(value);
    else if (strcmp(key, "border_color") == 0)
      cfg->border_color = _parse_color(value);
  }

  fclose(f);
}
