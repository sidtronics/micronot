#include "server.h"
#include "unotd.h"
#include <assert.h>
#include <locale.h>
#include <poll.h>

Unotd unotd = {

    .open = {.head = NULL, .tail = NULL, .lock = PTHREAD_MUTEX_INITIALIZER},

    .wait = {.head = NULL, .tail = NULL, .lock = PTHREAD_MUTEX_INITIALIZER},

    .notification_opened = PTHREAD_COND_INITIALIZER,

    .config =
        {
            .x_padding = 2,
            .y_padding = 2,
            .x_offset = 5,
            .y_offset = 10,
            .spacing = 5,
            .border_size = 2,
            .gap_size = 7,
            .indicator_size = 14.0,
            .timeout = 60,
            .background_color = 0x000000,
            .foreground_color = 0x00FF00,
            .border_color = 0x00FF00,
            .origin = UNOT_ORIGIN_BOTTOM_RIGHT,
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
                                    const char *m, char *i, int to) {

  Notification *new_not = notification_list_append(&unotd->open, NULL);
  new_not->window = 0;
  new_not->frame = NULL;
  new_not->txt_font = NULL;
  new_not->ind_color = NULL;
  new_not->txt_color = NULL;
  new_not->timeout = to;
  new_not->type = t;
  new_not->text = strdup(m);
  new_not->indicator = i;

  return new_not;
}

void *poll_loop(void *arg) {

  Unotd *unotd = (Unotd *)arg;

  while (1) {
    int poll_count = poll(unotd->set.fds, unotd->set.count, -1);

    if (poll_count == -1) {
      perror("poll");
      exit(1);
    }

    server_process_connections(unotd);
  }

  return NULL;
}

int main() {

  setlocale(LC_ALL, "en_US.utf8");
  unotd.display = XOpenDisplay(NULL);
  unotd.txt_font = XftFontOpenName(unotd.display, DefaultScreen(unotd.display),
                                   "FiraCodeNerdFontPropo:style=Bold:size=10");

  allocate_color(unotd.display, unotd.config.foreground_color,
                 &unotd.ind_color);
  allocate_color(unotd.display, unotd.config.foreground_color,
                 &unotd.txt_color);

  server_init(&unotd);

  sem_init(&unotd.notification_count, 0, 0);

  pthread_t poll_thread;
  if (pthread_create(&poll_thread, NULL, poll_loop, &unotd) != 0) {
    perror("pthread_create");
    exit(1);
  }

  //    append_new_not(&unotd, UNOT_TYPE_SPINNER, "Ascii Spinner",
  //                   "$[-]$[\\]$[|]$[/]$$[O]$[X]$$", 20);
  //
  //    append_new_not(&unotd, UNOT_TYPE_SPINNER, "Updating system",
  //                   "|||||||||||:style=Bold", 20);
  //
  //    append_new_not(&unotd, UNOT_TYPE_MESSAGE, "Stay Hydrated!",
  //    "|💧||:size=12", 10);
  //
  //    append_new_not(&unotd, UNOT_TYPE_SPINNER, "Syncing mirrors",
  //                   "|🌍|🌎|🌏||✔️|✖️||", 20);
  //
  //    append_new_not(&unotd, UNOT_TYPE_MESSAGE, "Now playing: FATRAT",
  //    "|🎶||", 30);
  //
  //    Notification *ker = append_new_not(
  //        &unotd, UNOT_TYPE_SPINNER, "Compiling Kernel",
  //        "|▱▱▱|▰▱▱|▰▰▱|▰▰▰|▰▰▱|▰▱▱|▱▱▱||S|F||", -1);
  //    ker->ind_color = (XftColor *)"0x235486";
  //
  //    Notification *not =
  //        append_new_not(&unotd, UNOT_TYPE_MESSAGE, "WARN: Low Battery",
  //        "|||", -1);
  //    not->ind_color = (XftColor *)"0xffa500";
  //    not->txt_color = (XftColor *)"0xffa500";

  sem_wait(&unotd.notification_count);

  while (1) {
    unotd_handle_events(&unotd);
    notification_list_foreach(&unotd.open, NULL, unotd_update_visitor, &unotd);
    XSync(unotd.display, False);

    struct timespec ts = {0, 50 * 1000 * 1000};
    nanosleep(&ts, NULL);
  }
}
