#include "indicator.h"
#include "utils.h"
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

  if (!end || end == str + 2)
    return false;

  return true;
}

const char *indicator_resolve_name(Config *config, const char *name_str,
                                   const char **hints) {

  assert(name_str && "indicator_resolve_name: name_str");
  assert(config && "indicator_resolve_name: config");

  const char delim = *name_str;
  const char *name_beg = name_str + 2;
  const char *name_end = strstr(name_beg, (const char[]){delim, delim, '\0'});
  *hints = name_end + 2;
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

  const char *hints = NULL;

  if (indicator_validate_cust_str(str)) {
    ind->str = strdup(str);
    ind->custom_string = true;
  }

  else if (indicator_validate_name_str(str)) {
    ind->str = indicator_resolve_name(config, str, &hints);
    if (!ind->str)
      return false;
    ind->custom_string = false;
  }

  else
    return false;

  FcCharSet *charset = FcCharSetCreate();
  FcPattern *pattern, *matched_pattern;

  const char delim = *ind->str;
  const char *p = ind->str + 1;
  ind->frame_count = 0;

  while (1) {

    wchar_t wc;
    int n = mbtowc(&wc, p, MB_CUR_MAX);
    assert(n > 0 && "indicator_init: mbtowc() failed");

    FcCharSetAddChar(charset, (FcChar32)wc);
    p += n;

    if (*p == delim) {
      p++;
      ind->frame_count++;
      if (strchr(p, delim) == NULL)
        break;
    }
  }

  if (!hints || !*hints)
    hints = p;

  pattern = FcNameParse((FcChar8 *)hints);
  assert(pattern && "FcNameParse failed");

  // Turns out FcNameParse doesn't support custom
  // fields, so you can't really have ":fps=4" in
  // your hints, which would have been much cleaner honestly.
  //
  // The current workaround is to repurpose a standard
  // property that is rarely used directly. I came up 
  // with this:
  //
  // dpi = "delay per indicator"

  double dpi = 0;
  FcPatternGetDouble(pattern, FC_DPI, 0, &dpi);
  ind->ms_per_frame = CLAMP(dpi, 150, 60 * 1000);
  FcPatternDel(pattern, FC_DPI);

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
  ind->frame_size = (size_t)(strchr(ind->frame, delim) - ind->frame);

  FcCharSetDestroy(charset);
  FcPatternDestroy(pattern);

  return true;
}

void indicator_free_str(Display *dpy, Indicator *ind) {

  if (ind->font)
    XftFontClose(dpy, ind->font);

  if (ind->custom_string)
    free((void *)ind->str);
}

void indicator_step_frame(Indicator *ind) {

  assert(ind && "indicator_parse: ind");

  const char delim = *ind->str;
  ind->frame = strchr(ind->frame, delim) + 1;
  const char *end = strchr(ind->frame, delim);
  if (!end) {
    ind->frame = ind->str + 1;
    end = strchr(ind->frame, delim);
  }

  ind->frame_size = (size_t)(end - ind->frame);
}
