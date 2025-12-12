#ifndef MICRONOT_INDICATOR_H
#define MICRONOT_INDICATOR_H

#include <X11/Xft/Xft.h>
#include <stdbool.h>

typedef enum _IndicatorType {
  INDICATOR_TYPE_ICON,
  INDICATOR_TYPE_SPINNER
} IndicatorType;

typedef struct _Indicator {

  IndicatorType type;

  const char *start;
  const char *frame;

  XftFont *font;
  XftColor *color;

} Indicator;

void indicator_init(Display *dpy, Indicator *ind, const char *str, double size);
bool indicator_validate(const char *str);
size_t indicator_next(Indicator *ind);
void indicator_free(Display *dpy, Indicator *ind);

#endif
