static u32 ddr_perf_alloc_counter(struct ddr_pmu *pmu, int event)
{
	int i;

	/*
	 * Always map cycle event to counter 0
	 * Cycles counter is dedicated for cycle event
	 * can't used for the other events
	 */
	if (event == EVENT_CYCLES_ID) {
		if (pmu->events[EVENT_CYCLES_COUNTER] == NULL)
			return EVENT_CYCLES_COUNTER;
		else
			return -ENOENT;
	}

	for (i = 1; i < NUM_COUNTERS; i++) {
		if (pmu->events[i] == NULL)
			return i;
	}

	return -ENOENT;
}
