// Tiny 5x5, monospace pixel font for microcontrollers.
// Based of lcamtuf's font-inline.h, but even smaller and with somewhat
// nicer kerning. See https://maurycyz.com/projects/mcufont/
//
// Intended to be rendered on a 6 by 6 grid.
//
// Designed for low-memory, low-resolution devices. A proportional
// font would use less horizontal space, but make layout a lot more
// complicated. (and updates slower)
// 
// Included charaters:
//   A-Z a-z 0-9 ' - + = . , ! ? :
//
// 5x5 is the smallest size that preserves letters like 'E' and 'M'.
// Going smaller is possible, but at the cost of legibility, and upper/lower
// case distinctiveness (most of my lowercase is actually 4x5):
//
// 1x1 = Impossible. 
// 2x2 = Impossible. Only 15 (non-blank) symbols are possible.
// 3x3 = Nearly illegible.
// 4x4 = Better, but still hard to read. Upper and lower case look similar.
// 5x5 = This. 
//
// Memory usage:
//   uppercase = 130  bytes
//   lowercase = 130  bytes
//   numbers   = 50   bytes
//   specials  = 40   bytes
//   total     = 350  bytes ~= 0.34 kB

#include <stdint.h>

// On the AVR, this is a macro that tells the compiler to put data in flash
// instead of RAM. On non-harvard architectures, the font should be normal data.
#ifndef PROGMEM
#define PROGMEM
#endif

// Comment out to save space if you don't need some charaters
#define FNT_UPPER   1
#define FNT_LOWER   1
#define FNT_NUMBERS 1
#define FNT_SYMBOLS 1
// Comment to omit utility function.
#define FNT_UTIL    1 

// Utility function to map an ASCII charater to a bitmap.
//
// Return value: A pointer to 7 bytes, each containing a single row
// of pixels. Bits 4-0 should be rendered, MSB first. Bit 5 may be
// rendered as a built-in space between charaters.
#ifdef FNT_UTIL
const uint8_t* fnt_get_sym(char c);
#endif

#if FNT_UPPER
const uint8_t fnt_upper[26][5] PROGMEM = {
	{ /* A */
		0b01110,
		0b10001,
		0b11111,
		0b10001,
		0b10001,
	},
	{ /* B */
		0b11110,
		0b10001,
		0b11110,
		0b10001,
		0b11110,
	},
	{ /* C */
		0b01111,
		0b10000,
		0b10000,
		0b10000,
		0b01111,
	},
	{ /* D */
		0b11110,
		0b10001,
		0b10001,
		0b10001,
		0b11110,
	},
	{ /* E */
		0b11111,
		0b10000,
		0b11100,
		0b10000,
		0b11111,
	},
	{ /* F */
		0b11111,
		0b10000,
		0b11100,
		0b10000,
		0b10000,
	},
	{ /* G */
		0b01111,
		0b10000,
		0b10011,
		0b10001,
		0b01111,
	},
	{ /* H */
		0b10001,
		0b10001,
		0b11111,
		0b10001,
		0b10001,
	},
	{ /* I */
		0b11111,
		0b00100,
		0b00100,
		0b00100,
		0b11111,
	},
	{ /* J */
		0b11111,
		0b00010,
		0b00010,
		0b10010,
		0b01100,
	},
	{ /* K */
		0b10010,
		0b10100,
		0b11000,
		0b10100,
		0b10010,
	},
	{ /* L */
		0b10000,
		0b10000,
		0b10000,
		0b10000,
		0b11111,
	},
	{ /* M */
		0b11111,
		0b10101,
		0b10101,
		0b10001,
		0b10001,
	},
	{ /* N */
		0b10001,
		0b11001,
		0b10101,
		0b10011,
		0b10001,
	},
	{ /* O */
		0b01110,
		0b10001,
		0b10001,
		0b10001,
		0b01110,
	},
	{ /* P */
		0b11110,
		0b10001,
		0b11110,
		0b10000,
		0b10000,
	},
	{ /* Q */
		0b01110,
		0b10001,
		0b10001,
		0b10010,
		0b01101,
	},
	{ /* R */
		0b11110,
		0b10001,
		0b11110,
		0b10010,
		0b10001,
	},
	{ /* S */
		0b01111,
		0b10000,
		0b01110,
		0b00001,
		0b11110,
	},
	{ /* T */
		0b11111,
		0b00100,
		0b00100,
		0b00100,
		0b00100,
	},
	{ /* U */
		0b10001,
		0b10001,
		0b10001,
		0b10001,
		0b01110,
	},
	{ /* V */
		0b10001,
		0b10001,
		0b01010,
		0b01010,
		0b00100,
	},
	{ /* W */
		0b10001,
		0b10001,
		0b10101,
		0b10101,
		0b11011,
	},
	{ /* X */
		0b10001,
		0b01010,
		0b00100,
		0b01010,
		0b10001,
	},
	{ /* Y */
		0b10001,
		0b01010,
		0b00100,
		0b00100,
		0b00100,
	},
	{ /* Z */
		0b11111,
		0b00010,
		0b00100,
		0b01000,
		0b11111,
	}
};
#endif

#if FNT_LOWER
const uint8_t fnt_lower[26][5] PROGMEM = {
	{ /* a */
		0b00000,
		0b01111,
		0b10001,
		0b10001,
		0b01111,
	},
	{ /* b */
		0b10000,
		0b11110,
		0b10001,
		0b10001,
		0b11110,
	},
	{ /* c */
		0b00000,
		0b01111,
		0b10000,
		0b10000,
		0b01111,
	},
	{ /* d */
		0b00001,
		0b01111,
		0b10001,
		0b10001,
		0b01111,
	},
	{ /* e */
		0b00000,
		0b01110,
		0b11111,
		0b10000,
		0b01111,
	},
	{ /* f */
		0b00000,
		0b01111,
		0b10000,
		0b11110,
		0b10000,
	},
	{ /* g */
		0b00000,
		0b01110,
		0b11111,
		0b00001,
		0b11110,
	},
	{ /* h */
		0b10000,
		0b10000,
		0b11110,
		0b10001,
		0b10001,
	},
	{ /* i */
		0b00100,
		0b00000,
		0b01100,
		0b00100,
		0b01110,
	},
	{ /* j */
		0b00010,
		0b00000,
		0b00010,
		0b10010,
		0b01100,
	},
	{ /* k */
		0b10000,
		0b10000,
		0b10110,
		0b11000,
		0b10110,
	},
	{ /* l */
		0b00000,
		0b10000,
		0b10000,
		0b10000,
		0b01111,
	},
	{ /* m */
		0b00000,
		0b11110,
		0b10101,
		0b10101,
		0b10001,
	},
	{ /* n */
		0b00000,
		0b11110,
		0b10001,
		0b10001,
		0b10001,
	},
	{ /* o */
		0b00000,
		0b01110,
		0b10001,
		0b10001,
		0b01110,
	},
	{ /* p */
		0b00000,
		0b11110,
		0b10001,
		0b11110,
		0b10000,
	},
	{ /* q */
		0b00000,
		0b01111,
		0b10001,
		0b01111,
		0b00001,
	},
	{ /* r */
		0b00000,
		0b01110,
		0b10000,
		0b10000,
		0b10000,
	},
	{ /* s */
		0b00000,
		0b01110,
		0b11000,
		0b00110,
		0b11100,
	},
	{ /* t */
		0b00000,
		0b11111,
		0b00100,
		0b00100,
		0b00100,
	},
	{ /* u */
		0b00000,
		0b10001,
		0b10001,
		0b10001,
		0b01110,
	},
	{ /* v */
		0b00000,
		0b10001,
		0b10001,
		0b01010,
		0b00100,
	},
	{ /* w */
		0b00000,
		0b10001,
		0b10101,
		0b10101,
		0b01110,
	},
	{ /* x */
		0b00000,
		0b10010,
		0b01100,
		0b01100,
		0b10010,
	},
	{ /* y */
		0b00000,
		0b10010,
		0b01110,
		0b00010,
		0b01100,
	},
	{ /* z */
		0b00000,
		0b11110,
		0b00100,
		0b01000,
		0b11110,
	}
};
#endif

#if FNT_NUMBERS
const uint8_t fnt_digits[10][5] PROGMEM = {
	{ /* 0 */
		0b01110,
		0b10001,
		0b10101,
		0b10001,
		0b01110,
	},
	{ /* 1 */
		0b01100,
		0b10100,
		0b00100,
		0b00100,
		0b11111,
	},
	{ /* 2 */
		0b01110,
		0b10001,
		0b00110,
		0b01000,
		0b11111,
	},
	{ /* 3 */
		0b11111,
		0b00001,
		0b01110,
		0b00001,
		0b11110,
	},
	{ /* 4 */
		0b10010,
		0b10010,
		0b11111,
		0b00010,
		0b00010,
	},
	{ /* 5 */
		0b11111,
		0b10000,
		0b01110,
		0b00001,
		0b11110,
	},
	{ /* 6 */
		0b01110,
		0b10000,
		0b11110,
		0b10001,
		0b01110,
	},
	{ /* 7 */
		0b11111,
		0b00010,
		0b00100,
		0b01000,
		0b01000,
	},
	{ /* 8 */
		0b01110,
		0b10001,
		0b01110,
		0b10001,
		0b01110,
	},
	{ /* 9 */
		0b11111,
		0b10001,
		0b11111,
		0b00001,
		0b00001,
	},
};
#endif

#define SYM_MINUS  0
#define SYM_PLUS   1
#define SYM_DOT    2
#define SYM_COMMA  3
#define SYM_QMARK  4
#define SYM_EXMARK 5
#define SYM_COLON  6
#define SYM_EQ     7
#define SYM_TICK   8

#if FNT_SYMBOLS
const uint8_t fnt_other[9][5] PROGMEM = {
	{ /* - */
		0b00000,
		0b00000,
		0b01110,
		0b00000,
		0b00000,
	},
	{ /* + */
		0b00000,
		0b00100,
		0b01110,
		0b00100,
		0b00000,
	},
	{ /* . */
		0b00000,
		0b00000,
		0b00000,
		0b00000,
		0b01000,
	},
	{ /* , */
		0b00000,
		0b00000,
		0b00000,
		0b00100,
		0b01000,
	},
	{ /* ? */
		0b01110,
		0b10001,
		0b00110,
		0b00000,
		0b00100,
	},
	{ /* ! */
		0b00100,
		0b00100,
		0b00100,
		0b00000,
		0b00100,
	},
	{ /* : */
		0b00000,
		0b01000,
		0b00000,
		0b01000,
		0b00000,
	},
	{ /* = */
		0b00000,
		0b01110,
		0b00000,
		0b01110,
		0b00000,
	},
	{ /* ' */
		0b00100,
		0b00100,
		0b00000,
		0b00000,
		0b00000,
	},
};
#endif

#ifdef FNT_UTIL
const uint8_t* fnt_get_sym(char c) {
	#if FNT_UPPER
	if (c >= 'A' && c <= 'Z') return fnt_upper[c - 'A'];
	#endif
	#if FNT_LOWER
	if (c >= 'a' && c <= 'z') return fnt_lower[c - 'a'];
	#endif
	#if FNT_NUMBERS
	if (c >= '0' && c <= '9') return fnt_digits[c - '0'];
	#endif
	#if FNT_SYMBOLS
	if (c == '-')  return fnt_other[SYM_MINUS];
	if (c == '+')  return fnt_other[SYM_PLUS];
	if (c == '.')  return fnt_other[SYM_DOT];
	if (c == ',')  return fnt_other[SYM_COMMA];
	if (c == '?')  return fnt_other[SYM_QMARK];
	if (c == '!')  return fnt_other[SYM_EXMARK];
	if (c == ':')  return fnt_other[SYM_COLON];
	if (c == '=')  return fnt_other[SYM_EQ];
	if (c == '\'') return fnt_other[SYM_TICK];
	#endif
	return 0;
}
#endif
