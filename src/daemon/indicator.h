#ifndef MICRONOT_INDICATOR_H
#define MICRONOT_INDICATOR_H

#include <X11/Xft/Xft.h>
#include <stdbool.h>

typedef enum _IndicatorType {
  INDICATOR_TYPE_ICON = 'i',
  INDICATOR_TYPE_SPINNER = 's'
} IndicatorType;

typedef struct _Indicator {

  IndicatorType type;

  char *start;
  char *frame;

  XftFont *font;
  XftColor *color;

} Indicator;

void indicator_init(Indicator *ind, char *ind_str);
void indicator_update(Indicator *ind);
bool indicator_validate(const char *ind_str);

void indicator_resolve_font(Display *dpy, Indicator *ind, double size);
void indicator_free_font(Display *dpy, Indicator *ind);

#endif
