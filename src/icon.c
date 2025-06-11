#include "icon.h"

UnotIcon *UnotCreateIcon(Display *dpy, XftFont *font, const char *icon) {

  UnotIcon *res = (UnotIcon *)malloc(sizeof(UnotIcon));
  res->icon = icon;
  res->font = font;
  XftTextExtentsUtf8(dpy, font, (FcChar8 *)icon, strlen(icon), &res->extents);
  return res;
}

void UnotDestroyIcon(UnotIcon *icon) { free(icon); }
