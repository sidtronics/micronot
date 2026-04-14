#ifndef MICRONOT_INDICATOR_H
#define MICRONOT_INDICATOR_H

#include "config.h"
#include <X11/Xft/Xft.h>
#include <stdbool.h>

#ifdef strstr
#undef strstr
#endif

typedef struct _Indicator {

  const char *str;

  const char *frame;
  size_t frame_size;

  XftFont *font;
  XftColor *color;

  u_int16_t frame_count;

  bool custom_string;
  bool custom_color;

  u_int16_t x, y;

} Indicator;

bool indicator_validate_cust_str(const char *str);
bool indicator_validate_name_str(const char *str);

bool indicator_init_str(Display *dpy, Config *config, Indicator *ind,
                        const char *str);
void indicator_free_str(Display *dpy, Indicator *ind);

void indicator_step_frame(Indicator *ind);

#endif
