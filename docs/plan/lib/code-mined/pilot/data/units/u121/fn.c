static __cpuidle int imx6q_enter_wait(struct cpuidle_device *dev,
				      struct cpuidle_driver *drv, int index)
{
	raw_spin_lock(&cpuidle_lock);
	if (++num_idle_cpus == num_online_cpus())
		imx6_set_lpm(WAIT_UNCLOCKED);
	raw_spin_unlock(&cpuidle_lock);

	ct_cpuidle_enter();
	cpu_do_idle();
	ct_cpuidle_exit();

	raw_spin_lock(&cpuidle_lock);
	if (num_idle_cpus-- == num_online_cpus())
		imx6_set_lpm(WAIT_CLOCKED);
	raw_spin_unlock(&cpuidle_lock);

	return index;
}
