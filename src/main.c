#include "server.h"
#include "unotd.h"
#include <assert.h>
#include <locale.h>
#include <poll.h>

// Unotd unotd = {
//
//     .open = {.head = NULL, .tail = NULL},
//     .wait = {.head = NULL, .tail = NULL},
//
//     .nlist_lock = PTHREAD_MUTEX_INITIALIZER,
//     .nlist_empty = PTHREAD_COND_INITIALIZER,
//     .notif_open = PTHREAD_COND_INITIALIZER,
//
//     .config =
//         {
//             .x_padding = 2,
//             .y_padding = 2,
//             .x_offset = 5,
//             .y_offset = 10,
//             .spacing = 5,
//             .border_size = 2,
//             .gap_size = 7,
//             .indicator_size = 14.0,
//             .timeout = 60,
//             .bg_color = 0x000000,
//             .text_color = 0x00FF00,
//             .indicator_color = 0x0000FF,
//             .border_color = 0x00FF00,
//             .origin = UNOT_ORIGIN_BOTTOM_RIGHT,
//         },
// };

static void allocate_color(Display *dpy, unsigned long color, XftColor *res) {

  XRenderColor xrcolor = {.red = ((color >> 16) & 0xff) * 257,
                          .green = ((color >> 8) & 0xff) * 257,
                          .blue = (color & 0xff) * 257,
                          .alpha = 0xffff};

  int scr_nbr = DefaultScreen(dpy);

  XftColorAllocValue(dpy, DefaultVisual(dpy, scr_nbr),
                     DefaultColormap(dpy, scr_nbr), &xrcolor, res);
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

  Unotd unotd = {0};
  unotd.open = (NotificationList){.head = NULL, .tail = NULL};
  unotd.wait = (NotificationList){.head = NULL, .tail = NULL};
  pthread_mutex_init(&unotd.nlist_lock, NULL);
  pthread_cond_init(&unotd.nlist_empty, NULL);
  pthread_cond_init(&unotd.notif_open, NULL);

  setlocale(LC_ALL, "en_US.utf8");
  unotd.display = XOpenDisplay(NULL);
  unotd.txt_font = XftFontOpenName(unotd.display, DefaultScreen(unotd.display),
                                   "FiraCodeNerdFontPropo:style=Bold:size=8");

  config_load(&unotd.config, "../unotrc");

  allocate_color(unotd.display, unotd.config.text_color, &unotd.txt_color);

  allocate_color(unotd.display, unotd.config.indicator_color, &unotd.ind_color);

  server_init(&unotd);

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

  while (1) {

    pthread_mutex_lock(&unotd.nlist_lock);

    while (nlist_empty(&unotd.open)) {
      pthread_cond_wait(&unotd.nlist_empty, &unotd.nlist_lock);
    }

    unotd_update_notifications(&unotd);

    pthread_mutex_unlock(&unotd.nlist_lock);

    XSync(unotd.display, False);

    struct timespec ts = {0, 50 * 1000 * 1000};
    nanosleep(&ts, NULL);
  }
}
