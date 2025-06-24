#ifndef UNOT_UTILS_H
#define UNOT_UTILS_H

#include <X11/Xft/Xft.h>
#include <assert.h>
#include <fontconfig/fontconfig.h>

#include "config.h"
#include "notification.h"

XftFont *utils_match_indicator_font(Display *dpy, const char *indicator,
                                    const char *hints);

XftColor *utils_allocate_custom_color(Display *dpy, const char *color_str);

void utils_calculate_notification_layout(Display *dpy, Config *config,
                                         Notification *target);

void utils_reposition_notification(Display *dpy, Config *config,
                                   Notification *previous,
                                   Notification *target);

#endif
