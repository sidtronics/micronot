#ifndef UNOT_UTILS_H
#define UNOT_UTILS_H

#include <X11/Xft/Xft.h>
#include <assert.h>
#include <fontconfig/fontconfig.h>
#include <stdbool.h>
#include <stdint.h>

#include "config.h"
#include "notification.h"

#if 1
#define ASSERT(x) assert((x))
#else
#define ASSERT(...) ((void)0)
#endif

#define MAX(a, b) ((a) > (b) ? (a) : (b))

bool utils_parse_ul(const char *str, unsigned long *res, int base);
bool utils_parse_u16(const char *str, uint16_t *res, int base);
bool utils_parse_dbl(const char *str, double *res);

void utils_resolve_indicator_font(Display *dpy, double size,
                                  Notification *target);

XftColor* utils_allocate_color(Display *dpy, unsigned long color, XftColor *res);
XftColor* utils_allocate_color_s(Display *dpy, const char *color_str, XftColor *res);

void utils_calculate_notification_layout(Display *dpy, Config *config,
                                         Notification *target);

void utils_reposition_notification(Display *dpy, Config *config,
                                   Notification *prev, Notification *target);

void utils_transform_notification(Notification *target, unsigned long ret);

#endif
