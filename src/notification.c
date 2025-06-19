#include "notification.h"
#include <assert.h>
#include <fontconfig/fontconfig.h>
#include <stdbool.h>
#include <wchar.h>

#if 1
#define ASSERT(x) assert((x))
#else
#define ASSERT(...) ((void)0)
#endif

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define GET_WIN_COORD(scr, win, bor) (scr) - (win) - 2 * (bor)

static XftFont *_match_indicator_font(Display *dpy, Config *config,
                                      const char *indicator) {

  FcCharSet *charset = FcCharSetCreate();
  FcPattern *pattern = FcPatternCreate();
  FcPattern *matched_pattern;

  const char *p = indicator;

  while (*p) {

    wchar_t wc;
    int n = mbtowc(&wc, p, MB_CUR_MAX);
    ASSERT(n > 0 && "mbtowc failed");

    FcCharSetAddChar(charset, (FcChar32)wc);
    p += n;
    if (*p == 0 || *p == 30)
      p++;
  }

  FcConfigSubstitute(NULL, pattern, FcMatchPattern);
  FcDefaultSubstitute(pattern);

  FcResult result;
  matched_pattern = FcFontMatch(NULL, pattern, &result);
  ASSERT(result == FcResultMatch);

  XftFont *font = XftFontOpenPattern(dpy, matched_pattern);
  ASSERT(font && "cant open Xftfont");

  FcCharSetDestroy(charset);
  FcPatternDestroy(pattern);

  return font;
}

static void _calculate_indicator_extents(Display *dpy, XftFont *font,
                                         const char *indicator,
                                         XGlyphInfo *ext) {

  size_t ind_size;
  const char *p = indicator;
  for (size_t i = 0; i < 3; i++) {
    ind_size += strlen(p);
    p += (ind_size + 1);
  }

  XftTextExtentsUtf8(dpy, font, (FcChar8 *)indicator, ind_size, ext);
}

static Window _create_notification_window(Display *dpy, uint8_t win_h,
                                          uint8_t win_w, uint8_t bor_w,
                                          unsigned long bg_color,
                                          unsigned long bor_color) {

  Screen *screen = XDefaultScreenOfDisplay(dpy);
  u_int32_t scr_nbr = XScreenNumberOfScreen(screen);

  u_int32_t mask =
      CWBackPixel | CWBorderPixel | CWOverrideRedirect | CWEventMask;
  XSetWindowAttributes attrs;
  attrs.background_pixel = bg_color;
  attrs.border_pixel = bor_color;
  attrs.event_mask = ExposureMask | ButtonPressMask | StructureNotifyMask;
  attrs.override_redirect = 1;

  Window window = XCreateWindow(dpy, XRootWindowOfScreen(screen),
                                GET_WIN_COORD(screen->width, win_w, bor_w),
                                GET_WIN_COORD(screen->height, win_h, bor_w),
                                win_w, win_h, bor_w, CopyFromParent,
                                InputOutput, CopyFromParent, mask, &attrs);

  ASSERT(window && "_create_notification_window: failed to create window.");

  XSelectInput(dpy, window,
               ExposureMask | ButtonPressMask | StructureNotifyMask);
  XMapWindow(dpy, window);
  XSync(dpy, False);

  return window;
}

static void _draw_notification(Display *dpy, Notification *notification) {

  if (notification->type == UNOT_SPINNER) {

    const char *end = strchr(notification->indicator, 30);
    XftDrawStringUtf8(notification->draw, notification->ind_color,
                      notification->ind_font, notification->ind_x,
                      notification->ind_y, (FcChar8 *)notification->indicator,
                      (size_t)(end - notification->indicator));
    notification->frame = end + 1;
  }

  else if (notification->type == UNOT_MESSAGE) {

    XftDrawStringUtf8(notification->draw, notification->ind_color,
                      notification->ind_font, notification->ind_x,
                      notification->ind_y, (FcChar8 *)notification->indicator,
                      strlen(notification->indicator));
  }

  XftDrawStringUtf8(notification->draw, notification->msg_color,
                    notification->msg_font, notification->msg_x,
                    notification->msg_y, (FcChar8 *)notification->message,
                    strlen(notification->message));

  XFlush(dpy);
}

void notification_open(Display *dpy, Config *config,
                       Notification *notification) {

  ASSERT(notification->indicator &&
         "notification_open: indicator string not set");
  ASSERT(notification->ind_font &&
         "notification_open: ind_font string not set");
  ASSERT(notification->ind_color &&
         "notification_open: ind_color string not set");
  ASSERT(notification->message && "notification_open: message string not set");
  ASSERT(notification->msg_font &&
         "notification_open: msg_font string not set");
  ASSERT(notification->msg_color &&
         "notification_open: msg_color string not set");

  //  notification->ind_font =
  //      _match_indicator_font(dpy, config, notification->indicator);

  XGlyphInfo ind_extents;
  _calculate_indicator_extents(dpy, notification->ind_font,
                               notification->indicator, &ind_extents);

  XGlyphInfo extents;
  XftTextExtentsUtf8(dpy, notification->msg_font,
                     (FcChar8 *)notification->message,
                     strlen(notification->message), &extents);

  uint8_t x_padding = config->x_padding;
  uint8_t y_padding = config->y_padding;
  uint8_t spacing = config->spacing;
  uint8_t bor_w = config->border_thickness;

  uint8_t msg_w = extents.width;
  uint8_t ind_w = ind_extents.width;

  uint8_t max_h = MAX(extents.height, ind_extents.height);

  uint8_t win_w = ind_w + spacing + msg_w + x_padding * 2;
  uint8_t win_h = max_h + y_padding * 2;

  notification->ind_x = x_padding;
  notification->msg_x = x_padding + ind_w + spacing;
  notification->ind_y = ind_extents.y + (win_h - ind_extents.height) / 2;
  notification->msg_y = extents.y + (win_h - extents.height) / 2;

  Screen *screen = XDefaultScreenOfDisplay(dpy);
  u_int32_t scr_nbr = XScreenNumberOfScreen(screen);

  notification->window = _create_notification_window(
      dpy, win_h, win_w, bor_w, config->background_color, config->border_color);

  notification->draw =
      XftDrawCreate(dpy, notification->window, DefaultVisual(dpy, scr_nbr),
                    DefaultColormap(dpy, scr_nbr));

  XSelectInput(dpy, notification->window,
               ExposureMask | ButtonPressMask | StructureNotifyMask);

  XMapWindow(dpy, notification->window);
  XSync(dpy, False);

  _draw_notification(dpy, notification);

  clock_gettime(CLOCK_MONOTONIC, &notification->last_updated);
}

void notification_close(Display *dpy, Notification *notification) {

  XftDrawDestroy(notification->draw);
  XDestroyWindow(dpy, notification->window);
}

void notification_update(Display *dpy, Notification *notification) {

  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  long elapsed = (now.tv_sec - notification->last_updated.tv_sec) * 1000 +
                 (now.tv_nsec - notification->last_updated.tv_nsec) / 1000000;

  if (notification->type == UNOT_MESSAGE) {

    if (elapsed >= notification->timeout * 1000) {
      XUnmapWindow(dpy, notification->window);
    }

  }

  else if (notification->type == UNOT_SPINNER) {

    if (elapsed >= 200) {

      notification->current_frame =
          (notification->current_frame + 1) % notification->spinner->count;

      XClearArea(dpy, notification->window, 0, 0, notification->t_x, 0, False);

      const char *frame =
          notification->spinner->frames[notification->current_frame];
      XftDrawStringUtf8(notification->draw, &notification->color,
                        notification->spinner->font, notification->i_x,
                        notification->i_y, (FcChar8 *)frame, strlen(frame));

      XFlush(dpy);
      notification->last_updated = now;
    }
  }
}
