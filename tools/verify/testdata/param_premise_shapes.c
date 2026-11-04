/* SPDX-License-Identifier: AGPL-3.0-or-later */
/* SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au> */
/*
 * Every shape tools/verify/param_premise.py has to get right, in the
 * layout style(9) actually produces. Not compiled; read as text, which is
 * how the tool reads it.
 */

static int	shapes_probe(device_t);		/* a prototype, not a call */
static void	shapes_isr(void *);

int shapes_global;

/*
 * A comment block sits directly above the declarator with no blank line
 * between, so the walk up from the brace ends inside the comment and the
 * `static' is not the first word of what it collected.
 */
static int
commented(struct softc *sc, int flag)
{

	return (sc->x + flag);
}

/* A parameter that is itself a function pointer must not truncate the
 * list, and its name is inside the parentheses. */
static int
with_callback(struct softc *sc, int (*cmp)(const void *, const void *),
    size_t n)
{
/* A macro body can close its brace at column 0 in the middle of a
 * function.  lib/libc/stdio/vfprintf.c does it twice inside __vfprintf. */
#define	SHAPES_FLUSH() { \
	if (cmp(sc, &n))     \
		return (1);  \
}

	SHAPES_FLUSH();
	return (cmp(sc, &n));
}

TAILQ_HEAD_INITIALIZER(shapes_list);
RB_GENERATE_STATIC(shapes_tree, shapes_node, entry, shapes_cmp)

int
exported_one(struct softc *sc)
{

	return (commented(sc, 0));
}

static int
shapes_probe(device_t dev)
{

	return (0);
}

static void
shapes_isr(void *arg)
{

	shapes_global++;
}

static device_method_t shapes_methods[] = {
	DEVMETHOD(device_probe,	shapes_probe),
	DEVMETHOD_END
};
