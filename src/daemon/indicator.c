#include "indicator.h"
#include <assert.h>

void indicator_init(Display *dpy, Indicator *ind, const char *str,
                    double size) {

  FcCharSet *charset = FcCharSetCreate();
  FcPattern *pattern, *matched_pattern;

  int frame_count = 0;
  const char delim = *str;
  const char *p = str + 1;

  while (1) {

    wchar_t wc;
    int n = mbtowc(&wc, p, MB_CUR_MAX);
    assert(n > 0 && "indicator_init: mbtowc() failed");

    FcCharSetAddChar(charset, (FcChar32)wc);
    p += n;

    if (*p == delim) {
      p++;
      frame_count++;
      if (strchr(p, delim) == NULL)
        break;
    }
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
  assert(result == FcResultMatch && "indicator_init: failed matching font");

  ind->font = XftFontOpenPattern(dpy, matched_pattern);
  assert(ind->font && "indicator_init: couldn't open font");

  ind->start = str;
  ind->frame = str + 1;
  ind->type = frame_count > 1 ? INDICATOR_TYPE_SPINNER : INDICATOR_TYPE_ICON;

  FcCharSetDestroy(charset);
  FcPatternDestroy(pattern);
}

bool indicator_validate(const char *str) {

  assert(str && "indicator_validate: str");

  if (!*str)
    return false;

  const char delim = *str;
  const char *frame_beg = str;
  const char *frame_end;
  while ((frame_end = strchr(frame_beg + 1, delim))) {

    if (frame_end == frame_beg + 1)
      return false;

    frame_beg = frame_end;
  }

  return frame_beg != str;
}

size_t indicator_next(Indicator *ind) {

  assert(ind && "indicator_parse: ind");

  const char delim = *ind->start;
  ind->frame = strchr(ind->frame, delim) + 1;
  const char *end = strchr(ind->frame, delim);
  if (!end) {
    ind->frame = ind->start + 1;
    end = strchr(ind->frame, delim);
  }

  return (size_t)(end - ind->frame);
}

void indicator_free(Display *dpy, Indicator *ind) {
  XftFontClose(dpy, ind->font);
}
