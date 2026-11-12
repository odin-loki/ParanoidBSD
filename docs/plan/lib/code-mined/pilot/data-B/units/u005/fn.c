static int __init page_frag_test_init(void)
{
	struct task_struct *tsk_push, *tsk_pop;
	int last_pushed = 0, last_popped = 0;
	ktime_t start;
	u64 duration;
	int ret;

	page_frag_cache_init(&test_nc);
	atomic_set(&nthreads, 2);
	init_completion(&wait);

	if (test_alloc_len > PAGE_SIZE || test_alloc_len <= 0 ||
	    !cpu_active(test_push_cpu) || !cpu_active(test_pop_cpu))
		return -EINVAL;

	ret = ptr_ring_init(&ptr_ring, nr_objs, GFP_KERNEL);
	if (ret)
		return ret;

	tsk_push = kthread_create_on_cpu(page_frag_push_thread, &ptr_ring,
					 test_push_cpu, "page_frag_push");
	if (IS_ERR(tsk_push))
		return PTR_ERR(tsk_push);

	tsk_pop = kthread_create_on_cpu(page_frag_pop_thread, &ptr_ring,
					test_pop_cpu, "page_frag_pop");
	if (IS_ERR(tsk_pop)) {
		kthread_stop(tsk_push);
		return PTR_ERR(tsk_pop);
	}

	start = ktime_get();
	wake_up_process(tsk_push);
	wake_up_process(tsk_pop);

	pr_info("waiting for test to complete\n");

	while (!wait_for_completion_timeout(&wait, msecs_to_jiffies(10000))) {
		/* exit if there is no progress for push or pop size */
		if (last_pushed == test_pushed || last_popped == test_popped) {
			WARN_ONCE(true, TEST_FAILED_PREFIX "no progress\n");
			force_exit = true;
			continue;
		}

		last_pushed = test_pushed;
		last_popped = test_popped;
		pr_info("page_frag_test progress: pushed = %d, popped = %d\n",
			test_pushed, test_popped);
	}

	if (force_exit) {
		pr_err(TEST_FAILED_PREFIX "exit with error\n");
		goto out;
	}

	duration = (u64)ktime_us_delta(ktime_get(), start);
	pr_info("%d of iterations for %s testing took: %lluus\n", nr_test,
		test_align ? "aligned" : "non-aligned", duration);

out:
	ptr_ring_cleanup(&ptr_ring, NULL);
	page_frag_cache_drain(&test_nc);

	return -EAGAIN;
}
