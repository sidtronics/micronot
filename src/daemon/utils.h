#ifndef MICRONOT_UTILS_H
#define MICRONOT_UTILS_H

#include "notification.h"
#include <assert.h>
#include <stdint.h>

#define MATCH(a, b) (strcmp((a), (b)) == 0)
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(x, lo, hi) ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))

bool utils_parse_ul(const char *str, unsigned long *res, int base);
bool utils_parse_u16(const char *str, uint16_t *res, int base);
bool utils_parse_dbl(const char *str, double *res);

XftColor *utils_allocate_color(Display *dpy, unsigned long color,
                               XftColor *res);

void utils_deallocate_color(Display *dpy, XftColor *color);

void utils_calculate_notification_layout(Display *dpy, Config *config,
                                         Notification *target);

void utils_reposition_notification(Display *dpy, Config *config,
                                   Notification *prev, Notification *target);
#endif
