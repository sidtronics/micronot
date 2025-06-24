#ifndef UNOT_CONFIG_H
#define UNOT_CONFIG_H

#include <stdint.h>

typedef enum _Origin {

  UNOT_ORIGIN_TOP_RIGHT,
  UNOT_ORIGIN_TOP_LEFT,
  UNOT_ORIGIN_BOTTOM_RIGHT,
  UNOT_ORIGIN_BOTTOM_LEFT

} Origin;

typedef struct _Config {

  Origin origin;

  uint8_t x_padding;
  uint8_t y_padding;
  uint8_t x_offset;
  uint8_t y_offset;
  uint8_t spacing;
  uint8_t border_thickness;
  uint8_t gap_size;
  double indicator_size;
  unsigned long background_color;
  unsigned long foreground_color;
  unsigned long border_color;
} Config;

#endif
