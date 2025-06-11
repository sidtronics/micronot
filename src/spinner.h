#ifndef SPINNER_H
#define SPINNER_H

#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <stdint.h>

typedef struct _UnotSpinner {

  const char **frames;
  uint8_t count;
  XftFont *font;
  XGlyphInfo extents;
} UnotSpinner;

UnotSpinner *UnotCreateSpinner(Display *dpy, XftFont *font, const char **frames,
                               uint8_t count);

void UnotDestroySpinner(UnotSpinner *spinner);

#endif
