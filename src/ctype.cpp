#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "utils.hpp"

#define BLANK (1 << 0)
#define CTRL (1 << 1)
#define PUNCT (1 << 2)
#define ALNUM (1 << 3)
#define UPPER (1 << 8)
#define LOWER (1 << 9)
#define ALPHA (1 << 10)
#define DIGIT (1 << 11)
#define XDIGIT (1 << 12)
#define SPACE (1 << 13)
#define PRINT (1 << 14)
#define GRAPH (1 << 15)

static constexpr unsigned short ctype_b_arr[384] {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	CTRL, CTRL, CTRL, CTRL, CTRL, CTRL, CTRL, CTRL, CTRL, CTRL | SPACE | BLANK, CTRL | SPACE,
	CTRL | SPACE, CTRL | SPACE, CTRL | SPACE, CTRL, CTRL, CTRL, CTRL, CTRL, CTRL, CTRL, CTRL,
	CTRL, CTRL, CTRL, CTRL, CTRL, CTRL, CTRL, CTRL, CTRL, CTRL, SPACE | PRINT | BLANK,
	PUNCT | PRINT | GRAPH, PUNCT | PRINT | GRAPH, PUNCT | PRINT | GRAPH, PUNCT | PRINT | GRAPH, PUNCT | PRINT | GRAPH, PUNCT | PRINT | GRAPH,
	PUNCT | PRINT | GRAPH, PUNCT | PRINT | GRAPH, PUNCT | PRINT | GRAPH, PUNCT | PRINT | GRAPH, PUNCT | PRINT | GRAPH, PUNCT | PRINT | GRAPH,
	PUNCT | PRINT | GRAPH, PUNCT | PRINT | GRAPH, PUNCT | PRINT | GRAPH, DIGIT | XDIGIT | PRINT | GRAPH, DIGIT | XDIGIT | PRINT | GRAPH, DIGIT | XDIGIT | PRINT | GRAPH,
	DIGIT | XDIGIT | PRINT | GRAPH, DIGIT | XDIGIT | PRINT | GRAPH, DIGIT | XDIGIT | PRINT | GRAPH, DIGIT | XDIGIT | PRINT | GRAPH, DIGIT | XDIGIT | PRINT | GRAPH,
	DIGIT | XDIGIT | PRINT | GRAPH, DIGIT | XDIGIT | PRINT | GRAPH, PUNCT | PRINT | GRAPH, PUNCT | PRINT | GRAPH, PUNCT | PRINT | GRAPH, PUNCT | PRINT | GRAPH,
	PUNCT | PRINT | GRAPH, PUNCT | PRINT | GRAPH, PUNCT | PRINT | GRAPH, UPPER | XDIGIT | ALPHA | PRINT | GRAPH, UPPER | XDIGIT | ALPHA | PRINT | GRAPH,
	UPPER | XDIGIT | ALPHA | PRINT | GRAPH, UPPER | XDIGIT | ALPHA | PRINT | GRAPH, UPPER | XDIGIT | ALPHA | PRINT | GRAPH, UPPER | XDIGIT | ALPHA | PRINT | GRAPH,
	UPPER | ALPHA | PRINT | GRAPH, UPPER | ALPHA | PRINT | GRAPH, UPPER | ALPHA | PRINT | GRAPH, UPPER | ALPHA | PRINT | GRAPH, UPPER | ALPHA | PRINT | GRAPH,
	UPPER | ALPHA | PRINT | GRAPH, UPPER | ALPHA | PRINT | GRAPH, UPPER | ALPHA | PRINT | GRAPH, UPPER | ALPHA | PRINT | GRAPH, UPPER | ALPHA | PRINT | GRAPH, UPPER | ALPHA | PRINT | GRAPH,
	UPPER | ALPHA | PRINT | GRAPH, UPPER | ALPHA | PRINT | GRAPH, UPPER | ALPHA | PRINT | GRAPH, UPPER | ALPHA | PRINT | GRAPH, UPPER | ALPHA | PRINT | GRAPH, UPPER | ALPHA | PRINT | GRAPH,
	UPPER | ALPHA | PRINT | GRAPH, UPPER | ALPHA | PRINT | GRAPH, UPPER | ALPHA | PRINT | GRAPH, PUNCT | PRINT | GRAPH, PUNCT | PRINT | GRAPH, PUNCT | PRINT | GRAPH,
	PUNCT | PRINT | GRAPH, PUNCT | PRINT | GRAPH, PUNCT | PRINT | GRAPH, LOWER | XDIGIT | ALPHA | PRINT | GRAPH, LOWER | XDIGIT | ALPHA | PRINT | GRAPH, LOWER | XDIGIT | ALPHA | PRINT | GRAPH,
	LOWER | XDIGIT | ALPHA | PRINT | GRAPH, LOWER | XDIGIT | ALPHA | PRINT | GRAPH, LOWER | XDIGIT | ALPHA | PRINT | GRAPH, LOWER | ALPHA | PRINT | GRAPH, LOWER | ALPHA | PRINT | GRAPH,
	LOWER | ALPHA | PRINT | GRAPH, LOWER | ALPHA | PRINT | GRAPH, LOWER | ALPHA | PRINT | GRAPH, LOWER | ALPHA | PRINT | GRAPH, LOWER | ALPHA | PRINT | GRAPH, LOWER | ALPHA | PRINT | GRAPH, LOWER | ALPHA | PRINT | GRAPH,
	LOWER | ALPHA | PRINT | GRAPH, LOWER | ALPHA | PRINT | GRAPH, LOWER | ALPHA | PRINT | GRAPH, LOWER | ALPHA | PRINT | GRAPH, LOWER | ALPHA | PRINT | GRAPH, LOWER | ALPHA | PRINT | GRAPH, LOWER | ALPHA | PRINT | GRAPH, LOWER | ALPHA | PRINT | GRAPH,
	LOWER | ALPHA | PRINT | GRAPH, LOWER | ALPHA | PRINT | GRAPH, LOWER | ALPHA | PRINT | GRAPH, PUNCT | PRINT | GRAPH, PUNCT | PRINT | GRAPH, PUNCT | PRINT | GRAPH, PUNCT | PRINT | GRAPH, CTRL
};

static thread_local const unsigned short *ctype_b_loc = ctype_b_arr + 128;

EXPORT const unsigned short **__ctype_b_loc() {
	return &ctype_b_loc;
}

static constinit int32_t ctype_tolower_arr[384] {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, EOF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-',
	'.', '/', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';',
	'<', '=', '>', '?', '@', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
	'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y',
	'z', '[', '\\', ']', '^', '_', '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
	'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y',
	'z', '{', '|', '}', '~'
};

static thread_local int32_t *ctype_tolower_loc = ctype_tolower_arr + 128;

EXPORT int32_t **__ctype_tolower_loc() {
	return &ctype_tolower_loc;
}

static constinit int32_t ctype_toupper_arr[384] {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, EOF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-',
	'.', '/', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';',
	'<', '=', '>', '?', '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
	'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',
	'Z', '[', '\\', ']', '^', '_', '`', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
	'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',
	'Z', '{', '|', '}', '~'
};

static thread_local int32_t *ctype_toupper_loc = ctype_toupper_arr + 128;

EXPORT int32_t **__ctype_toupper_loc() {
	return &ctype_toupper_loc;
}

EXPORT size_t __ctype_get_mb_cur_max() {
	return 4;
}
