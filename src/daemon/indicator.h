#ifndef MICRONOT_INDICATOR_H
#define MICRONOT_INDICATOR_H

#include "config.h"
#include <X11/Xft/Xft.h>
#include <stdbool.h>

typedef enum _IndicatorType : u_int16_t {
  INDICATOR_TYPE_ICON,
  INDICATOR_TYPE_SPINNER
} IndicatorType;

typedef enum _IndicatorString {
  INDICATOR_STR_RAW,
  INDICATOR_STR_NAME,
  INDICATOR_STR_INVALID
} IndicatorString;

typedef struct _Indicator {

  const char *str;
  const char *frame;

  XftFont *font;
  XftColor *color;

  IndicatorType type;

  bool custom_string;
  bool custom_color;

  u_int16_t x, y;

} Indicator;

bool indicator_validate_raw_str(const char *str);
bool indicator_validate_name_str(const char *str);
IndicatorString indicator_validate(const char *str);

bool indicator_init_str(Display *dpy, Config *config, Indicator *ind, const char *str);
size_t indicator_step_frame(Indicator *ind);
void indicator_free_str(Display *dpy, Indicator *ind);

#endif
