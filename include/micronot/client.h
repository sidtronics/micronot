#ifndef MICRONOT_CLIENT_H
#define MICRONOT_CLIENT_H

#include <stdbool.h>

int unot_get_connection(const char *sock_path);

bool unot_notify_message(int conn, const char *text, const char *ind_str,
                         const char *txt_font, unsigned long timeout,
                         unsigned long ind_fg_color,
                         unsigned long txt_fg_color);

bool unot_notify_spinner(int conn, const char *text, const char *ind_str,
                         const char *txt_font, unsigned long timeout,
                         unsigned long ind_fg_color, unsigned long txt_fg_color,
                         unsigned long *wid);

bool unot_notify_spinner_n(int conn, const char *text, const char *ind_name,
                           const char *txt_font, unsigned long timeout,
                           unsigned long ind_fg_color,
                           unsigned long txt_fg_color, unsigned long *wid);

bool unot_return_spinner(int conn, unsigned long *wid);

void unot_close_connection(int conn);

#endif // !MICRONOT_CLIENT_H
