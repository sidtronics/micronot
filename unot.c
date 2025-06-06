#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <assert.h>
#include <unistd.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define GET_WIN_COORD(scr, win, bor) (scr) - (win) - 2 * (bor)

Bool handle_events(Display *display, Window window) {
  XEvent e;
  while (XPending(display)) {
    XNextEvent(display, &e);
    switch (e.type) {

    case Expose:
      break;
    case ButtonPress:
      if (e.xbutton.button == Button1) {
        XUnmapWindow(display, window);
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
  const char *icon = argv[2];
  assert(strlen(icon) == 6 * 3 && "icon error");

  Display *display = XOpenDisplay(NULL);

  Screen *screen = XDefaultScreenOfDisplay(display);
  u_int32_t scr_nbr = DefaultScreen(display);

  XftFont *font = XftFontOpenName(display, scr_nbr,
                                  "FiraCodeNerdFontMono:style=Bold:size=8");
  XftFont *i_font = XftFontOpenName(display, scr_nbr,
                                    "FiraCodeNerdFontMono:style=Bold:size=10");

  XGlyphInfo extents;
  XGlyphInfo i_extents;
  XftTextExtentsUtf8(display, font, (FcChar8 *)text, strlen(text), &extents);
  XftTextExtentsUtf8(display, i_font, (FcChar8 *)icon, strlen(icon),
                     &i_extents);

  assert(font && i_font && "font error");

  u_int32_t t_w = extents.xOff;
  u_int32_t i_w = i_extents.xOff / 6;

  u_int32_t h = MAX(extents.height, i_extents.height);

  u_int32_t x_padding = 5;
  u_int32_t y_padding = 3;
  u_int32_t spacing = 10;
  u_int32_t win_w = i_w + spacing + t_w + x_padding * 2;
  u_int32_t win_h = h + y_padding * 2;
  u_int32_t bor_w = 2;

  u_int32_t mask =
      CWBackPixel | CWBorderPixel | CWOverrideRedirect | CWEventMask;
  XSetWindowAttributes attrs;
  attrs.background_pixel = screen->black_pixel;
  attrs.border_pixel = 0x00FF00;
  attrs.event_mask = ExposureMask | ButtonPressMask;
  attrs.override_redirect = 1;

  Window window = XCreateWindow(display, XRootWindowOfScreen(screen),
                                GET_WIN_COORD(screen->width, win_w, bor_w),
                                GET_WIN_COORD(screen->height, win_h, bor_w),
                                win_w, win_h, bor_w, CopyFromParent,
                                InputOutput, CopyFromParent, mask, &attrs);

  XMapWindow(display, window);
  XSync(display, False);

  XftDraw *draw =
      XftDrawCreate(display, window, DefaultVisual(display, scr_nbr),
                    DefaultColormap(display, scr_nbr));

  XftColor color;
  XRenderColor xrcolor = {
      .red = 0, .green = 0xffff, .blue = 0, .alpha = 0xffff};
  XftColorAllocValue(display, DefaultVisual(display, scr_nbr),
                     DefaultColormap(display, scr_nbr), &xrcolor, &color);

  XSelectInput(display, window, ExposureMask | ButtonPressMask);

  u_int32_t i_x = x_padding;
  u_int32_t t_x = i_x + i_w + spacing;
  u_int32_t i_y, t_y;
  if (i_extents.height >= extents.height) {
    i_y = y_padding + i_extents.y;
    t_y = y_padding + (i_extents.height - extents.height) / 2 + extents.y;
  } else {
    t_y = y_padding + extents.y;
    i_y = y_padding + (extents.height - i_extents.height) / 2 + i_extents.y;
  }

  XftDrawStringUtf8(draw, &color, font, t_x, t_y, (XftChar8 *)text,
                    strlen(text));
  int i = 0;
  while (1) {

    if (handle_events(display, window))
      break;

    XClearArea(display, window, 0, 0, x_padding + 10, 0, False);
    XftDrawStringUtf8(draw, &color, i_font, i_x, i_y, (XftChar8 *)&icon[i * 3],
                      3);

    XFlush(display);
    usleep(200 * 1000);
    i = (i + 1) % 6;
  }

  XCloseDisplay(display);
}
