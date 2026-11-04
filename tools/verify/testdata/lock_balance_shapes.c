/*
 * The shapes lock_balance.py has to tell apart. Every function here is
 * named for what it is, and the test asserts exactly which lines are
 * reported.
 */

void mtx_lock(int *);
void mtx_unlock(int *);
void work(void);
int  release_and_test(int *);
void panic(const char *);
extern int lk, other;

/* LEAK at the `return (2)': the early exit forgot the unlock. */
int
leaks_on_an_early_exit(int seq)
{
	mtx_lock(&lk);
	if (seq == 0) {
		mtx_unlock(&lk);
		return (0);
	}
	if (seq > 10) {
		if (seq > 100)
			return (2);
		mtx_unlock(&lk);
		return (1);
	}
	mtx_unlock(&lk);
	return (3);
}

/* Not a leak: the contract is to hand the lock to the caller. */
int
returns_locked_on_purpose(int seq)
{
	mtx_lock(&lk);
	return (seq);
}

/* Not a leak: both arms release, so the fall-through does not hold. */
int
both_arms_release(int seq)
{
	mtx_lock(&lk);
	if (seq) {
		mtx_unlock(&lk);
		return (1);
	} else {
		mtx_unlock(&lk);
	}
	return (0);
}

/* Not a leak: entered holding it, dropped it for work, took it back. */
int
reacquires_for_its_caller(int seq)
{
	mtx_unlock(&lk);
	work();
	mtx_lock(&lk);
	if (seq)
		return (1);
	return (0);
}

/* Not a leak: the callee in the guard released it. */
int
hands_off_in_the_guard(int seq)
{
	mtx_lock(&lk);
	if (release_and_test(&lk)) {
		return (1);
	}
	mtx_unlock(&lk);
	return (0);
}

/* Not a leak: the return is unreachable. */
int
panics_first(int seq)
{
	mtx_lock(&lk);
	switch (seq) {
	case 0:
		mtx_unlock(&lk);
		return (0);
	default:
		panic("no");
		return (1);
	}
}

/* Not a leak: a switch arm starts from the switch's entry state. */
int
switch_arm_does_not_leak_into_the_next(int seq)
{
	switch (seq) {
	case 0:
		mtx_lock(&lk);
		if (seq)
			mtx_unlock(&lk);
		mtx_unlock(&lk);
		break;
	default:
		return (1);
	}
	return (0);
}

/* LEAK at the `return (1)': the release wraps, and must still be seen. */
int
the_release_is_wrapped_across_two_lines(int seq)
{
	mtx_lock(&lk);
	if (seq == 1)
		return (1);
	if (seq == 2) {
		mtx_unlock(
		    &lk);
		return (2);
	}
	mtx_unlock(&lk);
	return (0);
}
