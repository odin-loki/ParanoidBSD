/* Reference logic from hbsd/src/lib/libthr/thread/thr_rwlock.c (dual-world). */
typedef unsigned long thread_id;

struct rwlock {
	thread_id writer;
	unsigned readers;
};

int
rwlock_tryrdlock(struct rwlock *l)
{
	if (l == NULL)
		return -1;
	if (l->writer != 0)
		return 1;
	++l->readers;
	return 0;
}

int
rwlock_trywrlock(struct rwlock *l, thread_id self)
{
	if (l == NULL)
		return -1;
	if (l->writer != 0 || l->readers > 0)
		return 1;
	l->writer = self;
	return 0;
}
