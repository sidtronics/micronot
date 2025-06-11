#include "spinner.h"

UnotSpinner *UnotCreateSpinner(Display *dpy, XftFont *font, const char **frames,
                               uint8_t count) {

  UnotSpinner *spinner = (UnotSpinner *)malloc(sizeof(UnotSpinner));
  spinner->frames = frames;
  spinner->count = count;
  spinner->font = font;
  spinner->extents = (XGlyphInfo){0, 0, 0, 0, 0, 0};

  XGlyphInfo ext;
  for (int i = 0; i < count + 2; i++) {
    XftTextExtentsUtf8(dpy, font, (FcChar8 *)frames[i], strlen(frames[i]),
                       &ext);
    if (spinner->extents.height < ext.height)
      spinner->extents.height = ext.height;
    if (spinner->extents.width < ext.width)
      spinner->extents.width = ext.width;
    if (spinner->extents.y < ext.y)
      spinner->extents.y = ext.y;
  }

  return spinner;
}

void UnotDestroySpinner(UnotSpinner *spinner) { free(spinner); }
