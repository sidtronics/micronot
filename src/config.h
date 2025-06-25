#ifndef UNOT_CONFIG_H
#define UNOT_CONFIG_H

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
  unsigned long background_color;
  unsigned long foreground_color;
  unsigned long border_color;
} Config;

#endif
