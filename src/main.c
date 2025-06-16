#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "icon.h"
#include "notification.h"

Bool handle_events(Display *display) {
  XEvent e;
  while (XPending(display)) {
    XNextEvent(display, &e);
    switch (e.type) {

    case Expose:
      break;
    case ButtonPress:
      if (e.xbutton.button == Button1) {
        XUnmapWindow(display, e.xbutton.window);
        return True;
      }
      break;
    }
  }
  return False;
}

int main(int argc, char **argv) {

  assert(argc == 3 && "argument error");
  const char *text = argv[1];
  const char *ico = argv[2];
  //  assert(strlen(icon) == 6 * 3 && "icon error");

  Display *display = XOpenDisplay(NULL);
  assert(display);

  u_int32_t scr_nbr = DefaultScreen(display);

  XftFont *font = XftFontOpenName(display, scr_nbr,
                                  "FiraCodeNerdFontMono:style=Bold:size=8");
  XftFont *ifont = XftFontOpenName(display, scr_nbr,
                                   "FiraCodeNerdFontMono:style=Bold:size=12");

  assert(font && "font error");

  Config config = (Config){5, 5, 5, 2, 0x000000, 0x00ff00, 0x00ff00};

  Spinner *spinner = CreateSpinner(
      display, ifont,
      (const char *[]){"", "", "", "", "", "", "", ""},
      6);

  assert(spinner);

  Icon *icon = CreateIcon(display, ifont, ico);

  Notification *not = notification_open(display, &config, font, text,
                                               UNOT_SPINNER, spinner, 5);

  while (1) {
    if (handle_events(display))
      break;

    notification_update(display, not);

    struct timespec ts = {0, 10 * 1000000}; // 10ms
    nanosleep(&ts, NULL);
  }

  XCloseDisplay(display);
}
