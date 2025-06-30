#include "utils.h"
#include <stdbool.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

bool utils_match_window(Notification *node, void *data) {

  Window target = (Window)(uintptr_t)data;
  return node->window == target;
}

static const char *_prepare_hints(const char *hints, double size, char *buf,
                                  size_t len) {

  if (hints && strstr(hints, ":size=")) {
    return hints;
  }

  if (!hints || *hints == '\0') {
    snprintf(buf, len, ":size=%.2f", size);
  } else {
    snprintf(buf, len, "%s:size=%.2f", hints, size);
  }

  return buf;
}

void utils_resolve_indicator_font(Display *dpy, double size,
                                  Notification *target) {

  FcCharSet *charset = FcCharSetCreate();
  FcPattern *pattern, *matched_pattern;

  const char delim = *target->indicator;
  const char *p = target->indicator + 1;
  bool traversed_frames = false;
  while (*p) {

    wchar_t wc;
    int n = mbtowc(&wc, p, MB_CUR_MAX);
    ASSERT(n > 0 && "utils_resolve_indicator_font: mbtowc() failed");

    FcCharSetAddChar(charset, (FcChar32)wc);
    p += n;

    if (*p == delim)
      p++;

    if (*p == delim) {
      if (target->type == UNOT_TYPE_MESSAGE || traversed_frames)
        break;
      else
        traversed_frames = true;
    }
  }

  ASSERT(*p && "utils_resolve_indicator_font: indicator string malformed");

  char hints_buf[64];
  const char *hints = _prepare_hints(p + 1, size, hints_buf, sizeof(hints_buf));

  pattern = FcNameParse((FcChar8 *)hints);
  FcPatternAddCharSet(pattern, FC_CHARSET, charset);
  FcConfigSubstitute(NULL, pattern, FcMatchPattern);
  FcDefaultSubstitute(pattern);

  FcResult result;
  matched_pattern = FcFontMatch(NULL, pattern, &result);
  // FcPatternPrint(matched_pattern);
  ASSERT(result == FcResultMatch &&
         "utils_resolve_indicator_font: failed matching font");

  target->ind_font = XftFontOpenPattern(dpy, matched_pattern);
  ASSERT(target->ind_font && "utils_resolve_indicator_font: cant open font");

  FcCharSetDestroy(charset);
  FcPatternDestroy(pattern);
}

XftColor *utils_allocate_custom_color(Display *dpy, const char *color_str) {

  unsigned long color = strtoul(color_str, NULL, 16);
  // ASSERT(color && "_allocate_custom_color: failed to parse color string");

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

  unsigned short ind_max_height = 0;
  unsigned short ind_max_width = 0;
  short ind_max_y = 0;

  const char delim = *target->indicator;
  const char *p = target->indicator + 1;
  bool traversed_frames = false;
  while (*p) {

    const char *end = strchrnul(p, delim);

    XGlyphInfo temp;
    XftTextExtentsUtf8(dpy, target->ind_font, (FcChar8 *)p, (size_t)(end - p),
                       &temp);

    if (ind_max_height < temp.height)
      ind_max_height = temp.height;

    if (ind_max_width < temp.width)
      ind_max_width = temp.width;

    if (ind_max_y < temp.y)
      ind_max_y = temp.y;

    p = end;

    if (*p == delim)
      p++;

    if (*p == delim) {
      if (target->type == UNOT_TYPE_MESSAGE || traversed_frames)
        break;
      else
        traversed_frames = true;
    }
  }

  ASSERT(*p &&
         "utils_calculate_notification_layout: malformed indicator string");

  XGlyphInfo extents;
  XftTextExtentsUtf8(dpy, target->txt_font, (FcChar8 *)target->text,
                     strlen(target->text), &extents);

  u_int16_t x_padding = config->x_padding;
  u_int16_t y_padding = config->y_padding;
  u_int16_t spacing = config->spacing;
  u_int16_t bor_w = config->border_size;
  u_int16_t txt_w = extents.width;
  u_int16_t ind_w = ind_max_width;
  u_int16_t max_h = MAX(extents.height, ind_max_height);

  target->win_w = ind_w + spacing + txt_w + x_padding * 2;
  target->win_h = max_h + y_padding * 2;

  target->ind_x = x_padding;
  target->ind_y = ind_max_y + (target->win_h - ind_max_height) / 2;

  target->txt_x = x_padding + ind_w + spacing;
  target->txt_y = extents.y + (target->win_h - extents.height) / 2;
}

void utils_reposition_notification(Display *dpy, Config *config,
                                   Notification *previous,
                                   Notification *target) {

  Screen *screen = DefaultScreenOfDisplay(dpy);

  u_int16_t bor_w = config->border_size;
  u_int16_t gap_size = config->gap_size;
  u_int16_t x_offset = config->x_offset;
  u_int16_t y_offset = config->y_offset;

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

void utils_transform_notification(Notification *target, unsigned long ret) {

  ASSERT(target->type != UNOT_TYPE_MESSAGE &&
         "utils_transform_notification: incompatible type");

  const char delim = *target->indicator;
  char *p;
  for (p = (target->indicator + 1); *p != delim && *p != 0;
       p = (strchrnul(p, delim) + 1)) {
  }

  ASSERT(*p && "utils_transform_notification: malformed indicator string");

  if (ret != 0)
    p = strchrnul(p + 1, delim);

  target->indicator = p;
  target->frame = p + 1;
  target->type = UNOT_TYPE_MESSAGE;
  clock_gettime(CLOCK_MONOTONIC, &target->start_time);
}
