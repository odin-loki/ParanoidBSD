/*
 * A cut-down stand-in for lib/libc/include/namespace.h: the rename that
 * hides libc's public entry points behind private names before any
 * system header declares them.
 */
#define		err				_err
#define		quit				_quit
