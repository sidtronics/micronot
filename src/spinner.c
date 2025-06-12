#include "spinner.h"

Spinner *CreateSpinner(Display *dpy, XftFont *font, const char **frames,
                               uint8_t count) {

  Spinner *spinner = (Spinner *)malloc(sizeof(Spinner));
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

void DestroySpinner(Spinner *spinner) { free(spinner); }
