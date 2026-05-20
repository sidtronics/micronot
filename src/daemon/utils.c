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

void utils_deallocate_color(Display *dpy, XftColor *color) {

  XftColorFree(dpy, DefaultVisual(dpy, DefaultScreen(dpy)),
               DefaultColormap(dpy, DefaultScreen(dpy)), color);
  free(color);
}

void _utils_calculate_notification_layout_woi(Display *dpy, Config *config,
                                              Notification *target) {
  int x_padding = config->x_padding;
  int y_padding = config->y_padding;

  XftFont *txt_font = target->txt_font;
  int txt_descent = txt_font->descent;
  int txt_height = txt_font->height;

  XGlyphInfo extents;
  XftTextExtentsUtf8(dpy, txt_font, (FcChar8 *)target->txt, strlen(target->txt),
                     &extents);

  target->txt_x = x_padding;
  target->txt_y = y_padding + (txt_height - txt_descent);

  target->win_w = x_padding * 2 + extents.xOff;
  target->win_h = txt_height + y_padding * 2;
}

void _utils_calculate_notification_layout_ind(Display *dpy, Config *config,
                                              Notification *target) {
  int x_padding = config->x_padding;
  int y_padding = config->y_padding;
  int spacing = config->spacing;

  XftFont *ind_font = target->ind.font;
  XftFont *txt_font = target->txt_font;
  int txt_descent = txt_font->descent;
  int txt_height = txt_font->height;

  unsigned short ind_max_width = 0;
  unsigned short ind_max_height = 0;
  short ind_max_y = INT16_MIN;

  const char delim = *target->ind.str;
  const char *p = target->ind.str + 1;
  const char *end;
  while ((end = strchr(p, delim)) != NULL) {
    XGlyphInfo temp;
    XftTextExtentsUtf8(dpy, ind_font, (FcChar8 *)p, (size_t)(end - p), &temp);
    if (ind_max_width < temp.xOff)
      ind_max_width = temp.xOff;
    if (ind_max_height < temp.height)
      ind_max_height = temp.height;
    if (ind_max_y < temp.y)
      ind_max_y = temp.y;
    p = end + 1;
  }

  XGlyphInfo extents;
  XftTextExtentsUtf8(dpy, txt_font, (FcChar8 *)target->txt, strlen(target->txt),
                     &extents);

  int content_h = MAX(txt_height, ind_max_height);

  target->ind.x = x_padding;
  target->ind.y = y_padding + (content_h - ind_max_height) / 2 + ind_max_y;

  target->txt_x = x_padding + spacing + ind_max_width;
  target->txt_y = y_padding + (content_h - txt_height) / 2 + (txt_height - txt_descent);

  target->win_w = x_padding * 2 + ind_max_width + spacing + extents.xOff;
  target->win_h = content_h + y_padding * 2;
}

void utils_calculate_notification_layout(Display *dpy, Config *config,
                                         Notification *target) {

  if (indicator_exists(&target->ind))
    _utils_calculate_notification_layout_ind(dpy, config, target);
  else
    _utils_calculate_notification_layout_woi(dpy, config, target);
}

void utils_reposition_notification(Display *dpy, Config *config,
                                   Notification *prev, Notification *target) {
  Screen *screen = DefaultScreenOfDisplay(dpy);

  const u_int16_t bor = config->border_size;
  const u_int16_t gap = config->gap_size;
  const u_int16_t x_off = config->x_offset;
  const u_int16_t y_off = config->y_offset;
  const u_int16_t bw = 2 * bor;

  /* horizontal placement */
  if (config->origin == UNOT_ORIGIN_TOP_RIGHT ||
      config->origin == UNOT_ORIGIN_BOTTOM_RIGHT) {
    target->win_x = screen->width - target->win_w - bw - x_off;
  } else {
    target->win_x = x_off;
  }

  /* vertical placement */
  switch (config->origin) {

  case UNOT_ORIGIN_TOP_RIGHT:
  case UNOT_ORIGIN_TOP_LEFT:
    target->win_y = prev ? prev->win_y + prev->win_h + bw + gap : y_off;
    break;

  case UNOT_ORIGIN_BOTTOM_RIGHT:
  case UNOT_ORIGIN_BOTTOM_LEFT:
    target->win_y = prev ? prev->win_y - gap - target->win_h - bw
                         : screen->height - target->win_h - bw - y_off;
    break;
  }
}
