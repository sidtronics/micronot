#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

typedef struct _UnotConfig {

  uint8_t x_padding;
  uint8_t y_padding;
  uint8_t spacing;
  uint8_t border_thickness;
  unsigned long background_color;
  unsigned long foreground_color;
  unsigned long border_color;
} UnotConfig;

#endif
