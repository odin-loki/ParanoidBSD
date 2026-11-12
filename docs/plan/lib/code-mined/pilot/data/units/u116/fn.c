static int rcu_torture_read_lock_trivial(void)
{
	preempt_disable();
	return 0;
}
