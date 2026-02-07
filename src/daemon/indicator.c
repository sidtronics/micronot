#include "indicator.h"
#include <assert.h>

bool indicator_validate_cust_str(const char *str) {

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

bool indicator_validate_name_str(const char *str) {

  assert(str && "indicator_validate: str");

  if (!*str || str[1] != str[0])
    return false;

  const char *end = strstr(str + 2, (const char[]){str[0], str[0], '\0'});

  if (!end || end == str + 2 || end[2] != '\0')
    return false;

  return true;
}

const char *indicator_resolve_name(Config *config, const char *name_str) {

  assert(name_str && "indicator_resolve_name: name_str");
  assert(config && "indicator_resolve_name: config");

  const char delim = *name_str;
  const char *name_beg = name_str + 2;
  const char *name_end = strstr(name_beg, (const char[]){delim, delim, '\0'});
  size_t name_len = (size_t)(name_end - name_beg);

  for (size_t i = 0; i < config->indicators_count; i++) {
    const char *indicator_name = config->indicators[i];
    if (strncmp(indicator_name, name_beg, name_len) == 0 &&
        indicator_name[name_len] == '\0') {
      return indicator_name + name_len + 1;
    }
  }

  return NULL;
}

bool indicator_init_str(Display *dpy, Config *config, Indicator *ind,
                        const char *str) {

  if (indicator_validate_cust_str(str)) {
    ind->str = strdup(str);
    ind->custom_string = true;
  }

  else if (indicator_validate_name_str(str)) {
    ind->str = indicator_resolve_name(config, str);
    if (!ind->str)
      return false;
    ind->custom_string = false;
  }

  else
    return false;

  FcCharSet *charset = FcCharSetCreate();
  FcPattern *pattern, *matched_pattern;

  int frame_count = 0;
  const char delim = *ind->str;
  const char *p = ind->str + 1;

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

  pattern = FcNameParse((FcChar8 *)p);
  FcPatternAddDouble(pattern, FC_SIZE, config->indicator_size);
  FcPatternAddCharSet(pattern, FC_CHARSET, charset);
  FcConfigSubstitute(NULL, pattern, FcMatchPattern);
  FcDefaultSubstitute(pattern);

  FcResult result;
  matched_pattern = FcFontMatch(NULL, pattern, &result);
  assert(result == FcResultMatch && "indicator_init: failed matching font");

  ind->font = XftFontOpenPattern(dpy, matched_pattern);
  assert(ind->font && "indicator_init: couldn't open font");

  ind->frame = ind->str + 1;
  ind->type = frame_count > 1 ? INDICATOR_TYPE_SPINNER : INDICATOR_TYPE_ICON;

  FcCharSetDestroy(charset);
  FcPatternDestroy(pattern);

  return true;
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

void indicator_free_str(Display *dpy, Indicator *ind) {

  if (ind->font)
    XftFontClose(dpy, ind->font);

  if (ind->custom_string)
    free((void *)ind->str);
}
