#include "unotd.h"
#include <locale.h>

Unotd unotd = {

    .origin = UNOT_ORIGIN_BOTTOM_RIGHT,

    .open = {.head = NULL, .tail = NULL},

    .wait = {.head = NULL, .tail = NULL},

    .config =
        {
            .x_padding = 2,
            .y_padding = 2,
            .x_offset = 5,
            .y_offset = 10,
            .spacing = 5,
            .border_thickness = 2,
            .gap_size = 7,
            .indicator_size = 10.0,
            .background_color = 0x000000,
            .foreground_color = 0x00FF00,
            .border_color = 0x00FF00,
        },

};

static void allocate_color(Display *dpy, unsigned long color, XftColor *res) {

  XRenderColor xrcolor = {.red = ((color >> 16) & 0xff) * 257,
                          .green = ((color >> 8) & 0xff) * 257,
                          .blue = (color & 0xff) * 257,
                          .alpha = 0xffff};

  int scr_nbr = DefaultScreen(dpy);

  XftColorAllocValue(dpy, DefaultVisual(dpy, scr_nbr),
                     DefaultColormap(dpy, scr_nbr), &xrcolor, res);
}

static Notification *append_new_not(Unotd *unotd, NotificationType t,
                                    const char *m, const char *i, int to) {

  Notification *new_not = notification_list_append(&unotd->open, NULL);
  new_not->window = 0;
  new_not->frame = NULL;
  new_not->msg_font = NULL;
  new_not->ind_color = NULL;
  new_not->msg_color = NULL;
  new_not->timeout = to;
  new_not->type = t;
  new_not->message = strdup(m);
  new_not->indicator = i;
  new_not->ind_font = unotd_resolve_indicator_font(unotd, new_not->indicator);
  assert(new_not->ind_font && "font not resolved");

  return new_not;
}

int main() {

  setlocale(LC_ALL, "en_US.utf8");
  unotd.display = XOpenDisplay(NULL);
  unotd.msg_font = XftFontOpenName(unotd.display, DefaultScreen(unotd.display),
                                   "FiraCodeNerdFontPropo:style=Bold:size=7");

  allocate_color(unotd.display, unotd.config.foreground_color,
                 &unotd.ind_color);
  allocate_color(unotd.display, unotd.config.foreground_color,
                 &unotd.msg_color);

  append_new_not(&unotd, UNOT_SPINNER, "Updating system",
                 "\x1E\x1E\x1E\x1E\x1E\0\0\0", 20);

  append_new_not(&unotd, UNOT_MESSAGE, "Reminder Drink Water!", "💧\0", 10);

  append_new_not(&unotd, UNOT_SPINNER, "Syncing mirrors",
                 "🌍\x1E🌎\x1E🌏\0✔️\0✖️\0", 20);

  append_new_not(&unotd, UNOT_MESSAGE, "Now playing: FATRAT", "🎶\0", 30);

  Notification *ker = append_new_not(
      &unotd, UNOT_SPINNER, "Compiling Kernel",
      "▱▱▱\x1e▰▱▱\x1e▰▰▱\x1e▰▰▰\x1e▰▰▱\x1e▰▱▱\x1e▱▱▱\0S\0F\0", -1);
  ker->ind_color = (XftColor *)"0x235486";

  Notification *not =
      append_new_not(&unotd, UNOT_MESSAGE, "WARN: Low Battery", "\0", -1);
  not->ind_color = (XftColor *)"0xffa500";
  not->msg_color = (XftColor *)"0xffa500";

  while (1) {
    unotd_handle_events(&unotd);
    unotd_update_notifications(&unotd);

    struct timespec ts = {0, 10 * 1000 * 1000};
    nanosleep(&ts, NULL);
  }
}
