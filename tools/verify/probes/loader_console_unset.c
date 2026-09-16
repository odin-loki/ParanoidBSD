/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * stand/: four of the five copies of the loader's getboothowto() pass
 * getenv("console") straight to a string function.
 *
 *   cbmc -DOLD tools/verify/probes/loader_console_unset.c
 *       -> 2 of 2 failed, FAILED
 *   cbmc       tools/verify/probes/loader_console_unset.c
 *       -> 0 of 2, SUCCESSFUL
 *
 *	common/metadata.c:58
 *	    if (!strcmp(getenv("console"), "comconsole"))
 *	efi/loader/main.c:1311
 *	    if (strcmp(getenv("console"), "efi") == 0) {
 *	i386/libi386/bootinfo.c:49, userboot/userboot/bootinfo.c:47
 *	    string = next = strdup(getenv("console"));
 *
 * getenv() returns NULL for a variable that is not set, and `unset
 * console' at the loader prompt is a thing a person can type - as is
 * an /boot/loader.conf or an EFI NextLoaderEnv that never sets it.
 * libsa's strdup() is malloc plus strlen, so the third shape is the
 * same dereference one call deeper, and the result is then handed to
 * strcmp() whether the strdup succeeded or not.
 *
 * efi/loader/bootinfo.c:82 is the fifth copy and it already reads
 *
 *	console = getenv("console");
 *	if (console != NULL) {
 *
 * which is the shape the other four now have.
 *
 * The assertion is that no string function is reached with NULL.
 */

typedef unsigned long size_t;

int nondet_int(void);

static const char *
getenv_console(void)
{
	/* Set, or not set. */
	return (nondet_int() ? "comconsole" : (const char *)0);
}

static int
my_strcmp(const char *a, const char *b)
{
	__CPROVER_assert(a != (const char *)0,
	    "strcmp is not reached with a NULL console");
	return (a == b);
}

static char *
my_strdup(const char *a)
{
	__CPROVER_assert(a != (const char *)0,
	    "strdup is not reached with a NULL console");
	return ((char *)a);
}

int
main(void)
{
	const char *console;
	char *string;

#ifdef OLD
	my_strcmp(getenv_console(), "comconsole");
	string = my_strdup(getenv_console());
#else
	console = getenv_console();
	if (console != (const char *)0)
		my_strcmp(console, "comconsole");

	console = getenv_console();
	string = (console != (const char *)0) ? my_strdup(console) : (char *)0;
#endif
	return (string != (char *)0);
}
