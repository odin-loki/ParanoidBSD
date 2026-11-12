static int __init
kfree_scale_init(void)
{
	int firsterr = 0;
	long i;
	unsigned long jif_start;
	unsigned long orig_jif;

	pr_alert("%s" SCALE_FLAG
		 "--- kfree_rcu_test: kfree_mult=%d kfree_by_call_rcu=%d kfree_nthreads=%d kfree_alloc_num=%d kfree_loops=%d kfree_rcu_test_double=%d kfree_rcu_test_single=%d\n",
		 scale_type, kfree_mult, kfree_by_call_rcu, kfree_nthreads, kfree_alloc_num, kfree_loops, kfree_rcu_test_double, kfree_rcu_test_single);

	// Also, do a quick self-test to ensure laziness is as much as
	// expected.
	if (kfree_by_call_rcu && !IS_ENABLED(CONFIG_RCU_LAZY)) {
		pr_alert("CONFIG_RCU_LAZY is disabled, falling back to kfree_rcu() for delayed RCU kfree'ing\n");
		kfree_by_call_rcu = 0;
	}

	if (kfree_by_call_rcu) {
		/* do a test to check the timeout. */
		orig_jif = rcu_get_jiffies_lazy_flush();

		rcu_set_jiffies_lazy_flush(2 * HZ);
		rcu_barrier();

		jif_start = jiffies;
		jiffies_at_lazy_cb = 0;
		call_rcu(&lazy_test1_rh, call_rcu_lazy_test1);

		smp_cond_load_relaxed(&rcu_lazy_test1_cb_called, VAL == 1);

		rcu_set_jiffies_lazy_flush(orig_jif);

		if (WARN_ON_ONCE(jiffies_at_lazy_cb - jif_start < 2 * HZ)) {
			pr_alert("ERROR: call_rcu() CBs are not being lazy as expected!\n");
			firsterr = -1;
			goto unwind;
		}

		if (WARN_ON_ONCE(jiffies_at_lazy_cb - jif_start > 3 * HZ)) {
			pr_alert("ERROR: call_rcu() CBs are being too lazy!\n");
			firsterr = -1;
			goto unwind;
		}
	}

	kfree_nrealthreads = compute_real(kfree_nthreads);
	/* Start up the kthreads. */
	if (shutdown) {
		init_waitqueue_head(&shutdown_wq);
		firsterr = torture_create_kthread(kfree_scale_shutdown, NULL,
						  shutdown_task);
		if (torture_init_error(firsterr))
			goto unwind;
		schedule_timeout_uninterruptible(1);
	}

	pr_alert("kfree object size=%zu, kfree_by_call_rcu=%d\n",
			kfree_mult * sizeof(struct kfree_obj),
			kfree_by_call_rcu);

	kfree_reader_tasks = kcalloc(kfree_nrealthreads, sizeof(kfree_reader_tasks[0]),
			       GFP_KERNEL);
	if (kfree_reader_tasks == NULL) {
		firsterr = -ENOMEM;
		goto unwind;
	}

	for (i = 0; i < kfree_nrealthreads; i++) {
		firsterr = torture_create_kthread(kfree_scale_thread, (void *)i,
						  kfree_reader_tasks[i]);
		if (torture_init_error(firsterr))
			goto unwind;
	}

	while (atomic_read(&n_kfree_scale_thread_started) < kfree_nrealthreads)
		schedule_timeout_uninterruptible(1);

	torture_init_end();
	return 0;

unwind:
	torture_init_end();
	kfree_scale_cleanup();
	return firsterr;
}
