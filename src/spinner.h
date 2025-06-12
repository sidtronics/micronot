#ifndef UNOT_SPINNER_H
#define UNOT_SPINNER_H

#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <stdint.h>

typedef struct _Spinner {

  const char **frames;
  uint8_t count;
  XftFont *font;
  XGlyphInfo extents;
} Spinner;

Spinner *CreateSpinner(Display *dpy, XftFont *font, const char **frames,
                               uint8_t count);

void DestroySpinner(Spinner *spinner);

#endif
