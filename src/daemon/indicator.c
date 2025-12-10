#include "indicator.h"
#include <assert.h>

void indicator_init(Indicator *ind, char *ind_str) {

  assert(ind && "indicator_parse: ind");
  assert(ind_str && "indicator_parse: ind_str");

  ind->type = *ind_str++;
  ind->start = ind_str++;
  ind->frame = ind_str;
}

void indicator_update(Indicator *ind) {

  assert(ind && "indicator_parse: ind");

  if (ind->type == INDICATOR_TYPE_SPINNER) {
    char delim = *ind->start;
    ind->frame = strchr(ind->frame, delim);
    if (strchr(ind->frame + 1, delim) == NULL)
      ind->frame = ind->start;
    ind->frame++;
  }
}

bool indicator_validate(const char *ind_str) {

  if (!ind_str || !*ind_str)
    return false;

  char type = *ind_str++;
  if (strchr("is", type) == NULL)
    return false;

  const char *delim_prev = ind_str++;
  char delim = *delim_prev;
  if (!delim)
    return false;

  char *delim_next;
  int frame_count = 0;
  while ((delim_next = strchr(ind_str, delim))) {

    if (delim_next == delim_prev + 1)
      return false;

    ind_str = delim_next + 1;
    delim_prev = delim_next;
    frame_count++;
  }

  return frame_count;
}

void indicator_resolve_font(Display *dpy, Indicator *ind, double size) {

  FcCharSet *charset = FcCharSetCreate();
  FcPattern *pattern, *matched_pattern;

  const char delim = *ind->start;
  const char *p = ind->start + 1;
  while (strchr(p, delim) != NULL) {

    wchar_t wc;
    int n = mbtowc(&wc, p, MB_CUR_MAX);
    assert(n > 0 && "indicator_resolve_font: mbtowc() failed");

    FcCharSetAddChar(charset, (FcChar32)wc);
    p += n;

    if (*p == delim)
      p++;
  }

  char hbuf[64];
  const char *hints = hbuf;
  if (strstr(p, ":size="))
    hints = p;
  else
    snprintf(hbuf, sizeof(hbuf), "%s:size=%.2f", p, size);

  pattern = FcNameParse((FcChar8 *)hints);
  FcPatternAddCharSet(pattern, FC_CHARSET, charset);
  FcConfigSubstitute(NULL, pattern, FcMatchPattern);
  FcDefaultSubstitute(pattern);

  FcResult result;
  matched_pattern = FcFontMatch(NULL, pattern, &result);
  // FcPatternPrint(matched_pattern);
  assert(result == FcResultMatch &&
         "indicator_resolve_font: failed matching font");

  ind->font = XftFontOpenPattern(dpy, matched_pattern);
  assert(ind->font && "indicator_resolve_font: couldn't open font");

  FcCharSetDestroy(charset);
  FcPatternDestroy(pattern);
}

void indicator_free_font(Display *dpy, Indicator *ind) {
  XftFontClose(dpy, ind->font);
};
