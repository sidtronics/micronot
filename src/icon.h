#ifndef ICON_H
#define ICON_H

#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <stdint.h>

typedef struct _UnotIcon {

  const char *icon;
  XftFont *font;
  XGlyphInfo extents;

} UnotIcon;

UnotIcon *UnotCreateIcon(Display *dpy, XftFont *font, const char *icon);

void UnotDestroyIcon(UnotIcon *icon);

#endif
