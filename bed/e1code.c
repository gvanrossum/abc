/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

/*
 * Compaction scheme for characters to save space in grammar tables
 * by combining characters with similar properties (digits, l.c. letters).
 */

#include "b.h"
#include "code.h"

Visible char code_array[RANGE];
Visible char invcode_array[RANGE];
Visible int lastcode= 0;

Visible Procedure initcodes() {
	int c;

	code_array['\n'] = ++lastcode;
	invcode_array[lastcode] = '\n';
	for (c = ' '; c <= '0'; ++c) {
		code_array[c] = ++lastcode;
		invcode_array[lastcode] = c;
	}
	for (; c <= '9'; ++c)
		code_array[c] = lastcode;
	for (; c <= 'a'; ++c) {
		code_array[c] = ++lastcode;
		invcode_array[lastcode] = c;
	}
	for (; c <= 'z'; ++c)
		code_array[c] = lastcode;
	for (; c < RANGE; ++c) {
		code_array[c] = ++lastcode;
		invcode_array[lastcode] = c;
	}
}
