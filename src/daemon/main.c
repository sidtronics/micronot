#include "server.h"
#include "utils.h"
#include <locale.h>
#include <poll.h>
#include <pthread.h>

int main() {

  ServerCtx sctx = {0};

  // Default config:
  sctx.config = (Config){.origin = UNOT_ORIGIN_BOTTOM_RIGHT,
                         .x_padding = 2,
                         .y_padding = 2,
                         .spacing = 2,
                         .border_size = 2,
                         .gap_size = 5,
                         .indicator_size = 10.0,
                         .timeout = 5,
                         .font = "monospace:size=9",
                         .bg_color = 0x000000,
                         .text_color = 0xFFFFFF,
                         .indicator_color = 0xFFFFFF,
                         .border_color = 0xFFFFFF};

  pthread_mutex_init(&sctx.nlist_lock, NULL);
  pthread_cond_init(&sctx.nlist_empty, NULL);
  pthread_cond_init(&sctx.notif_open, NULL);

  setlocale(LC_ALL, "en_US.utf8");
  sctx.display = XOpenDisplay(NULL);

  config_load(&sctx.config, "../unotrc");

  sctx.txt_font = XftFontOpenName(sctx.display, DefaultScreen(sctx.display),
                                  sctx.config.font);

  utils_allocate_color(sctx.display, sctx.config.text_color, &sctx.txt_color);
  utils_allocate_color(sctx.display, sctx.config.indicator_color,
                       &sctx.ind_color);

  server_init(&sctx);

  pthread_t poll_thread;
  if (pthread_create(&poll_thread, NULL, server_handle, &sctx) != 0) {
    perror("pthread_create");
    exit(1);
  }

  while (1) {

    pthread_mutex_lock(&sctx.nlist_lock);

    while (nlist_empty(&sctx.open)) {
      pthread_cond_wait(&sctx.nlist_empty, &sctx.nlist_lock);
    }
    server_update_notifications(&sctx);
    XSync(sctx.display, False);

    pthread_mutex_unlock(&sctx.nlist_lock);

    nanosleep(&(struct timespec){0, 50 * 1000 * 1000}, NULL);
  }
}
