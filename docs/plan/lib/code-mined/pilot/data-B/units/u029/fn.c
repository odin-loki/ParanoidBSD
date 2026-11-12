static int ucma_init_all(void)
{
	int i, ret = 0;

	if (!cma_dev_cnt) {
		ret = ucma_init();
		if (ret)
			return ret;
	}

	if (cma_init_cnt == cma_dev_cnt)
		return 0;

	pthread_mutex_lock(&mut);
	for (i = 0; i < cma_dev_cnt; i++) {
		ret = ucma_init_device(&cma_dev_array[i]);
		if (ret)
			break;
	}
	pthread_mutex_unlock(&mut);
	return ret;
}
