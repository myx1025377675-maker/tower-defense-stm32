#ifndef E08_LYRICS_H
#define E08_LYRICS_H

#include <stdint.h>

typedef struct
{
    uint32_t t_ms;
    const char *text;
} E08_LyricLine;

extern const E08_LyricLine g_baab_lyrics[];
extern const uint16_t g_baab_lyrics_count;
extern const uint16_t g_baab_bpm;

#endif

