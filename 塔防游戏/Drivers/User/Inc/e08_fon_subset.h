#ifndef E08_FON_SUBSET_H
#define E08_FON_SUBSET_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct { uint16_t code; uint8_t data[32]; } E08_Glyph16;
typedef struct { uint16_t code; uint8_t data[128]; } E08_Glyph32;

extern const E08_Glyph16 E08_GLYPHS_16[];
extern const uint32_t E08_GLYPHS_16_COUNT;
extern const E08_Glyph32 E08_GLYPHS_32[];
extern const uint32_t E08_GLYPHS_32_COUNT;

const uint8_t *E08_FindGlyph16(uint16_t code);
const uint8_t *E08_FindGlyph32(uint16_t code);

#ifdef __cplusplus
}
#endif

#endif
