#include "utils.h"
#include <errno.h>
#include <stdint.h>
#include <time.h>

bool utils_parse_ul(const char *str, unsigned long *res, int base) {

  assert(str && *str && "utils_parse_ul: str is NULL or empty");
  assert(res && "utils_parse_ul: res is NULL");

  char *endptr = NULL;
  errno = 0;

  unsigned long val = strtoul(str, &endptr, base);

  if (errno == ERANGE || *endptr != '\0')
    return false;

  *res = val;
  return true;
}

bool utils_parse_u16(const char *str, uint16_t *res, int base) {

  assert(res && "utils_parse_u16: res is NULL");

  unsigned long val;
  if (!utils_parse_ul(str, &val, base))
    return false;

  if (val > UINT16_MAX)
    return false;

  *res = (uint16_t)val;
  return true;
}

bool utils_parse_dbl(const char *str, double *res) {

  assert(str && *str && "utils_parse_ul: str is NULL or empty");
  assert(res && "utils_parse_ul: res is NULL");

  char *endptr = NULL;
  errno = 0;

  double val = strtod(str, &endptr);

  if (errno == ERANGE || *endptr != '\0')
    return false;

  *res = val;
  return true;
}

XftColor *utils_allocate_color(Display *dpy, unsigned long color,
                               XftColor *res) {

  XRenderColor xrcolor = {.red = ((color >> 16) & 0xff) * 257,
                          .green = ((color >> 8) & 0xff) * 257,
                          .blue = (color & 0xff) * 257,
                          .alpha = 0xffff};

  int scr_nbr = DefaultScreen(dpy);

  if (!res)
    res = malloc(sizeof(XftColor));

  XftColorAllocValue(dpy, DefaultVisual(dpy, scr_nbr),
                     DefaultColormap(dpy, scr_nbr), &xrcolor, res);

  return res;
}

XftColor *utils_allocate_color_s(Display *dpy, const char *color_str,
                                 XftColor *res) {

  unsigned long color;
  bool parsed = utils_parse_ul(color_str, &color, 10);
  assert(parsed && "utils_allocate_color: error parsing color string");

  return utils_allocate_color(dpy, color, res);
}

void utils_calculate_notification_layout(Display *dpy, Config *config,
                                         Notification *target) {

  unsigned short ind_max_height = 0;
  unsigned short ind_max_width = 0;
  short ind_max_y = 0;

  const char delim = *target->ind.start;
  const char *p = target->ind.start + 1;
  const char *end;
  while ((end = strchr(p, delim)) != NULL) {

    XGlyphInfo temp;
    XftTextExtentsUtf8(dpy, target->ind.font, (FcChar8 *)p, (size_t)(end - p),
                       &temp);

    if (ind_max_height < temp.height)
      ind_max_height = temp.height;

    if (ind_max_width < temp.width)
      ind_max_width = temp.width;

    if (ind_max_y < temp.y)
      ind_max_y = temp.y;

    p = end + 1;
  }

  XGlyphInfo extents;
  XftTextExtentsUtf8(dpy, target->txt_font, (FcChar8 *)target->txt,
                     strlen(target->txt), &extents);

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
                                   Notification *prev, Notification *target) {

  Screen *screen = DefaultScreenOfDisplay(dpy);

  u_int16_t bor_w = config->border_size;
  u_int16_t gap_size = config->gap_size;
  u_int16_t x_offset = config->x_offset;
  u_int16_t y_offset = config->y_offset;

  switch (config->origin) {

  case UNOT_ORIGIN_TOP_RIGHT:
    target->win_x = screen->width - target->win_w - 2 * bor_w - x_offset;
    target->win_y =
        prev
            ? (target->win_y = prev->win_y + prev->win_h + 2 * bor_w + gap_size)
            : y_offset;

    break;

  case UNOT_ORIGIN_TOP_LEFT:
    target->win_x = x_offset;
    target->win_y =
        prev
            ? (target->win_y = prev->win_y + prev->win_h + 2 * bor_w + gap_size)
            : y_offset;
    break;

  case UNOT_ORIGIN_BOTTOM_RIGHT:
    target->win_x = screen->width - target->win_w - 2 * bor_w - x_offset;
    target->win_y = prev ? (target->win_y = prev->win_y - gap_size -
                                            target->win_h - 2 * bor_w)
                         : (target->win_y = screen->height - target->win_h -
                                            2 * bor_w - y_offset);

    break;

  case UNOT_ORIGIN_BOTTOM_LEFT:
    target->win_x = x_offset;
    target->win_y = prev ? (target->win_y = prev->win_y - gap_size -
                                            target->win_h - 2 * bor_w)
                         : (target->win_y = screen->height - target->win_h -
                                            2 * bor_w - y_offset);
    break;
  }
}
