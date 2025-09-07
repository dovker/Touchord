#ifndef _inc_font
#define _inc_font

#include <stdint.h>

/*
 * Format
 * <height>, <width>, <additional spacing per char>, 
 * <first ascii char>, <last ascii char>,
 * <data>
 */

extern const uint8_t font_3x6[95*3 + 5];
extern const uint8_t font_8x5[96*5];
#endif
