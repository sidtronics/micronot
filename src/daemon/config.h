#ifndef MICRONOT_CONFIG_H
#define MICRONOT_CONFIG_H

#include <sys/types.h>

typedef enum _Origin {

  UNOT_ORIGIN_TOP_RIGHT,
  UNOT_ORIGIN_TOP_LEFT,
  UNOT_ORIGIN_BOTTOM_RIGHT,
  UNOT_ORIGIN_BOTTOM_LEFT

} Origin;

typedef struct _Config {

  Origin origin;

  u_int16_t x_padding;
  u_int16_t y_padding;
  u_int16_t x_offset;
  u_int16_t y_offset;
  u_int16_t spacing;
  u_int16_t border_size;
  u_int16_t gap_size;
  double indicator_size;
  const char* font;
  unsigned long timeout;
  unsigned long bg_color;
  unsigned long text_color;
  unsigned long indicator_color;
  unsigned long border_color;

  struct {
    char** indicators;
    size_t indicators_count;
  };

} Config;

void config_load(Config *cfg, const char *filename);

#endif
