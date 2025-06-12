#include "icon.h"

Icon *CreateIcon(Display *dpy, XftFont *font, const char *icon) {

  Icon *res = (Icon *)malloc(sizeof(Icon));
  res->icon = icon;
  res->font = font;
  XftTextExtentsUtf8(dpy, font, (FcChar8 *)icon, strlen(icon), &res->extents);
  return res;
}

void DestroyIcon(Icon *icon) { free(icon); }
