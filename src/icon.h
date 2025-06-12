#ifndef UNOT_ICON_H
#define UNOT_ICON_H

#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <stdint.h>

typedef struct _Icon {

  const char *icon;
  XftFont *font;
  XGlyphInfo extents;

} Icon;

Icon *CreateIcon(Display *dpy, XftFont *font, const char *icon);

void DestroyIcon(Icon *icon);

#endif
