#include "indicator.h"
#include <assert.h>

IndicatorString indicator_classify(const char *str) {

  assert(str && "indicator_classify: str");

  if (indicator_validate_str(str))
    return INDICATOR_STR_RAW;

  else if (*str && str[1] == str[0]) {

    const char *end = strstr(str + 2, (const char[]){str[0], str[0], '\0'});

    if (!end || end == str + 2 || end[2] != '\0')
      return INDICATOR_STR_INVALID;

    return INDICATOR_STR_NAME;
  }

  else
    return INDICATOR_STR_INVALID;
}

bool indicator_validate_str(const char *str) {

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

bool indicator_resolve_name(Config *config, const char **str) {

  assert(str && *str && "indicator_resolve_name: str");
  assert(config && "indicator_resolve_name: config");

  const char delim = (*str)[0];
  const char *name = *str + 2;
  const char *end = strstr(*str + 2, (const char[]){delim, delim, '\0'});
  size_t name_len = (size_t)(end - name);

  for (size_t i = 0; i < config->indicators_count; i++) {
    const char *indicator_name = config->indicators[i];
    if (strncmp(indicator_name, name, name_len) == 0 &&
        indicator_name[name_len] == '\0') {
      *str = indicator_name + name_len + 1;
      return true;
    }
  }

  return false;
}

void indicator_init(Display *dpy, Indicator *ind, double size) {

  FcCharSet *charset = FcCharSetCreate();
  FcPattern *pattern, *matched_pattern;

  int frame_count = 0;
  const char *str = ind->str;
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

  ind->frame = str + 1;
  ind->type = frame_count > 1 ? INDICATOR_TYPE_SPINNER : INDICATOR_TYPE_ICON;

  FcCharSetDestroy(charset);
  FcPatternDestroy(pattern);
}

size_t indicator_step_frame(Indicator *ind) {

  assert(ind && "indicator_parse: ind");

  const char delim = *ind->str;
  ind->frame = strchr(ind->frame, delim) + 1;
  const char *end = strchr(ind->frame, delim);
  if (!end) {
    ind->frame = ind->str + 1;
    end = strchr(ind->frame, delim);
  }

  return (size_t)(end - ind->frame);
}

void indicator_free(Display *dpy, Indicator *ind) {

  XftFontClose(dpy, ind->font);

  if (ind->custom_string)
    free((void *)ind->str);

  if (ind->custom_color) {
    XftColorFree(dpy, DefaultVisual(dpy, DefaultScreen(dpy)),
                 DefaultColormap(dpy, DefaultScreen(dpy)), ind->color);
    free(ind->color);
  }
}
