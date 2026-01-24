
// String, FlagIdentifier, Type, Name
#define HF_FIELDS                                                              \
  X("txt", UNOT_F_TEXT, HF_STRING_T, text)                                     \
  X("ind", UNOT_F_INDICATOR, HF_STRING_T, indicator)                           \
  X("tfn", UNOT_F_TEXT_FONT, HF_STRING_T, text_font)                           \
  X("ifg", UNOT_F_INDICATOR_FG, HF_INTEGER_T, indicator_fg)                    \
  X("tfg", UNOT_F_TEXT_FG, HF_INTEGER_T, text_fg)                              \
  X("tim", UNOT_F_TIMEOUT, HF_INTEGER_T, timeout)                              \
  X("nid", UNOT_F_NOTIFICATION_ID, HF_INTEGER_T, id)                           \

// String, Identifier, ValidFieldsMask
#define HF_HEADERS                                                             \
  X(                                                                           \
    "NOT",                                                                     \
    UNOT_H_NOTIFY,                                                             \
    UNOT_F_TEXT |                                                              \
    UNOT_F_TEXT_FG |                                                           \
    UNOT_F_TEXT_FONT |                                                         \
    UNOT_F_INDICATOR |                                                         \
    UNOT_F_INDICATOR_FG |                                                      \
    UNOT_F_TIMEOUT                                                             \
  )                                                                            \
                                                                               \
  X(                                                                           \
    "MOD",                                                                     \
    UNOT_H_MODIFY,                                                             \
    UNOT_F_NOTIFICATION_ID |                                                   \
    UNOT_F_TEXT |                                                              \
    UNOT_F_TEXT_FG |                                                           \
    UNOT_F_TEXT_FONT |                                                         \
    UNOT_F_INDICATOR |                                                         \
    UNOT_F_INDICATOR_FG |                                                      \
    UNOT_F_TIMEOUT                                                             \
  )                                                                            \
                                                                               \
  X(                                                                           \
    "OK",                                                                      \
    UNOT_H_OK,                                                                 \
    UNOT_F_NOTIFICATION_ID                                                     \
  )                                                                            \
                                                                               \
  X(                                                                           \
    "ERR",                                                                     \
    UNOT_H_ERROR,                                                              \
    0                                                                          \
  )
