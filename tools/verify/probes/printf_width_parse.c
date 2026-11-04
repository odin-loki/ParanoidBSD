/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * The three signed overflows in printf(3)'s conversion-specifier parse,
 * shared verbatim by lib/libc/stdio/vfprintf.c, vfwprintf.c and
 * xprintf.c.  All three are driven by the format string and its
 * arguments, which is to say by the caller.
 *
 *   cbmc -DOLD --unwind 12 --unwinding-assertions --signed-overflow-check \
 *       tools/verify/probes/printf_width_parse.c     -> FAILED
 *   cbmc        --unwind 12 --unwinding-assertions --signed-overflow-check \
 *       tools/verify/probes/printf_width_parse.c     -> SUCCESSFUL
 *
 *   - `width = -width' on the `%*d' path.  The comment above it quotes
 *     ANSI X3J11: "a negative field width argument is taken as a - flag
 *     followed by a positive field width".  INT_MIN has no positive
 *     counterpart, so printf("%*d", INT_MIN, 0) negates INT_MIN.
 *   - `n = 10 * n + to_digit(ch)' on the literal-width path, and the
 *     identical loop for the precision.  Ten digits are enough:
 *     printf("%9999999999d", 0).
 *
 * CBMC never reported any of this.  vfprintf() is one of the 649
 * TIMEOUTs -- a va_list, a jump table and a thousand lines -- so the
 * model checker gets no verdict at all on the file that every printf in
 * the system goes through.  This probe is the part of it that matters,
 * small enough to decide.
 */

#define INT_MAX		2147483647
#define INT_MIN		(-INT_MAX - 1)
#define EOF		(-1)
#define EOVERFLOW	84

#define to_digit(c)	((c) - '0')
#define is_digit(c)	((unsigned)to_digit(c) <= 9)

int nondet_int(void);
char nondet_char(void);

static int errno_;

/* The `%*d' path: GETASTER(width), then the negation. */
static int
aster_width(int arg, int *out, int *left)
{
	int width = arg;

	if (width >= 0) {
		*out = width;
		return (0);
	}
#ifdef OLD
	width = -width;				/* INT_MIN */
#else
	if (width == INT_MIN) {
		/*
		 * There is no positive field width to fall back to: the
		 * standard's rewriting is not available for this one
		 * value, and no such width could be honoured anyway.
		 */
		errno_ = EOVERFLOW;
		return (EOF);
	}
	width = -width;
#endif
	*left = 1;
	*out = width;
	return (0);
}

/* The literal-digit path, used for both the width and the precision. */
static int
digits(const char *fmt, int *out)
{
	int n = 0;
	int i;

	for (i = 0; i < 11 && is_digit(fmt[i]); i++) {
#ifndef OLD
		if (n > INT_MAX / 10 ||
		    (n == INT_MAX / 10 && to_digit(fmt[i]) > INT_MAX % 10)) {
			errno_ = EOVERFLOW;
			return (EOF);
		}
#endif
		n = 10 * n + to_digit(fmt[i]);
	}
	*out = n;
	return (0);
}

int
main(void)
{
	char fmt[12];
	int arg = nondet_int();
	int width = 0, prec = 0, left = 0;
	int i;

	for (i = 0; i < 11; i++) {
		fmt[i] = nondet_char();
		__CPROVER_assume(fmt[i] >= '0' && fmt[i] <= '9');
	}
	fmt[11] = '\0';

	if (aster_width(arg, &width, &left) == EOF)
		return (1);
	__CPROVER_assert(width >= 0, "the field width is non-negative");

	if (digits(fmt, &width) == EOF)
		return (1);
	if (digits(fmt, &prec) == EOF)
		return (1);
	__CPROVER_assert(width >= 0 && prec >= 0,
	    "the parsed width and precision are non-negative");
	return (0);
}
