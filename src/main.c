#include "server.h"
#include "unotd.h"
#include "utils.h"
#include <assert.h>
#include <locale.h>
#include <poll.h>
#include <time.h>

void *handle_server(void *arg) {

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
  pthread_mutex_init(&unotd.nlist_lock, NULL);
  pthread_cond_init(&unotd.nlist_empty, NULL);
  pthread_cond_init(&unotd.notif_open, NULL);

  setlocale(LC_ALL, "en_US.utf8");
  unotd.display = XOpenDisplay(NULL);
  unotd.txt_font = XftFontOpenName(unotd.display, DefaultScreen(unotd.display),
                                   "FiraCodeNerdFontPropo:style=Bold:size=8");

  config_load(&unotd.config, "../unotrc");

  utils_allocate_color(unotd.display, unotd.config.text_color,
                       &unotd.txt_color);
  utils_allocate_color(unotd.display, unotd.config.indicator_color,
                       &unotd.ind_color);

  server_init(&unotd);

  pthread_t poll_thread;
  if (pthread_create(&poll_thread, NULL, handle_server, &unotd) != 0) {
    perror("pthread_create");
    exit(1);
  }

  while (1) {

    pthread_mutex_lock(&unotd.nlist_lock);
    while (nlist_empty(&unotd.open)) {
      pthread_cond_wait(&unotd.nlist_empty, &unotd.nlist_lock);
    }
    unotd_update_notifications(&unotd);
    pthread_mutex_unlock(&unotd.nlist_lock);

    XSync(unotd.display, False);

    nanosleep(&(struct timespec){0, 50 * 1000 * 1000}, NULL);
  }
}
