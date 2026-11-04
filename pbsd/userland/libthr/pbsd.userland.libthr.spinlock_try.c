/* Reference logic from hbsd/src/lib/libthr/thread/thr_spinlock.c (dual-world). */

int
spin_trylock_ref(int locked, unsigned long owner, unsigned long self)
{
	if (locked && owner != self)
		return 1; /* EBUSY */
	return 0;
}
