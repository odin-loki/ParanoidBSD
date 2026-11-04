/* Reference logic from hbsd/src/lib/libthr/thread/thr_mutex.c (dual-world). */
typedef unsigned long thread_id;

struct mutex {
	thread_id owner;
	unsigned lock_count;
};

int
mutex_timedlock(struct mutex *m, thread_id self, unsigned ticks)
{
	(void)ticks;
	if (m == NULL || self == 0)
		return -1;
	if (m->owner == 0) {
		m->owner = self;
		m->lock_count = 1;
		return 0;
	}
	if (m->owner == self) {
		++m->lock_count;
		return 0;
	}
	return 1;
}
