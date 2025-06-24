#include "utils.h"

XftFont *utils_match_indicator_font(Display *dpy, const char *indicator,
                                    const char *hints) {

  FcCharSet *charset = FcCharSetCreate();
  FcPattern *pattern, *matched_pattern;

  const char *p = indicator;

  while (*p) {

    wchar_t wc;
    int n = mbtowc(&wc, p, MB_CUR_MAX);
    assert(n > 0 && "mbtowc failed");

    FcCharSetAddChar(charset, (FcChar32)wc);
    p += n;
    if (*p == 0 || *p == 30)
      p++;
  }

  pattern = FcNameParse((FcChar8 *)hints);
  FcPatternAddCharSet(pattern, FC_CHARSET, charset);
  FcConfigSubstitute(NULL, pattern, FcMatchPattern);
  FcDefaultSubstitute(pattern);

  FcResult result;
  matched_pattern = FcFontMatch(NULL, pattern, &result);
  //FcPatternPrint(matched_pattern);
  assert(result == FcResultMatch);

  XftFont *font = XftFontOpenPattern(dpy, matched_pattern);
  assert(font && "cant open Xftfont");

  FcCharSetDestroy(charset);
  FcPatternDestroy(pattern);

  return font;
}

XftColor *utils_allocate_custom_color(Display *dpy, const char *color_str) {

  unsigned long color = strtoul(color_str, NULL, 16);
  // assert(color && "_allocate_custom_color: failed to parse color string");

  XRenderColor xrcolor = {.red = ((color >> 16) & 0xff) * 257,
                          .green = ((color >> 8) & 0xff) * 257,
                          .blue = (color & 0xff) * 257,
                          .alpha = 0xffff};

  int scr_nbr = DefaultScreen(dpy);

  XftColor *res = malloc(sizeof(XftColor));

  XftColorAllocValue(dpy, DefaultVisual(dpy, scr_nbr),
                     DefaultColormap(dpy, scr_nbr), &xrcolor, res);

  return res;
}

void utils_calculate_notification_layout(Display *dpy, Config *config,
                                         Notification *target) {

  const char *p = target->indicator;
  XGlyphInfo ind_extents = {0};
  while (*p) {

    const char *end = strchrnul(p, 30);
    XGlyphInfo temp;
    XftTextExtentsUtf8(dpy, target->ind_font, (FcChar8 *)p, (size_t)(end - p),
                       &temp);

    if (ind_extents.height < temp.height)
      ind_extents.height = temp.height;
    if (ind_extents.width < temp.width)
      ind_extents.width = temp.width;
    if (ind_extents.y < temp.y)
      ind_extents.y = temp.y;

    p = end + 1;
  }

  XGlyphInfo extents;
  XftTextExtentsUtf8(dpy, target->msg_font, (FcChar8 *)target->message,
                     strlen(target->message), &extents);

  uint8_t x_padding = config->x_padding;
  uint8_t y_padding = config->y_padding;
  uint8_t spacing = config->spacing;
  uint8_t bor_w = config->border_thickness;

  uint8_t msg_w = extents.width;
  uint8_t ind_w = ind_extents.width;

  uint8_t max_h = (extents.height >= ind_extents.height) ? extents.height
                                                         : ind_extents.height;

  target->win_w = ind_w + spacing + msg_w + x_padding * 2;
  target->win_h = max_h + y_padding * 2;

  target->ind_x = x_padding;
  target->ind_y = ind_extents.y + (target->win_h - ind_extents.height) / 2;

  target->msg_x = x_padding + ind_w + spacing;
  target->msg_y = extents.y + (target->win_h - extents.height) / 2;
}

void utils_reposition_notification(Display *dpy, Config *config,
                                   Notification *previous,
                                   Notification *target) {

  Screen *screen = DefaultScreenOfDisplay(dpy);

  uint8_t bor_w = config->border_thickness;
  uint8_t gap_size = config->gap_size;
  uint8_t x_offset = config->x_offset;
  uint8_t y_offset = config->y_offset;

  switch (config->origin) {

  case UNOT_ORIGIN_TOP_RIGHT:
    target->win_x = screen->width - target->win_w - 2 * bor_w - x_offset;
    target->win_y = previous
                        ? (target->win_y = previous->win_y + previous->win_h +
                                           2 * bor_w + gap_size)
                        : y_offset;

    break;

  case UNOT_ORIGIN_TOP_LEFT:
    target->win_x = x_offset;
    target->win_y = previous
                        ? (target->win_y = previous->win_y + previous->win_h +
                                           2 * bor_w + gap_size)
                        : y_offset;
    break;

  case UNOT_ORIGIN_BOTTOM_RIGHT:
    target->win_x = screen->width - target->win_w - 2 * bor_w - x_offset;
    target->win_y = previous ? (target->win_y = previous->win_y - gap_size -
                                                target->win_h - 2 * bor_w)
                             : (target->win_y = screen->height - target->win_h -
                                                2 * bor_w - y_offset);

    break;

  case UNOT_ORIGIN_BOTTOM_LEFT:
    target->win_x = x_offset;
    target->win_y = previous ? (target->win_y = previous->win_y - gap_size -
                                                target->win_h - 2 * bor_w)
                             : (target->win_y = screen->height - target->win_h -
                                                2 * bor_w - y_offset);
    break;
  }
}
