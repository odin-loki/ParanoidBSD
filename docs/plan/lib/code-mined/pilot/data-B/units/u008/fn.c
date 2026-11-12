static int prepare_metric(const struct metric_expr *mexp,
			  const struct evsel *evsel,
			  struct expr_parse_ctx *pctx,
			  int cpu_idx, int thread_idx)
{
	struct evsel * const *metric_events = mexp->metric_events;
	struct metric_ref *metric_refs = mexp->metric_refs;

	for (int i = 0; metric_events[i]; i++) {
		char *n = strdup(evsel__metric_id(metric_events[i]));
		double val, ena, run;
		int source_count = evsel__source_count(metric_events[i]);
		int ret;
		struct perf_counts_values *old_count, *new_count;

		if (!n)
			return -ENOMEM;

		if (source_count == 0)
			source_count = 1;

		ret = evsel__ensure_counts(metric_events[i]);
		if (ret)
			return ret;

		/* Set up pointers to the old and newly read counter values. */
		old_count = perf_counts(metric_events[i]->prev_raw_counts, cpu_idx, thread_idx);
		new_count = perf_counts(metric_events[i]->counts, cpu_idx, thread_idx);
		/* Update the value in metric_events[i]->counts. */
		evsel__read_counter(metric_events[i], cpu_idx, thread_idx);

		val = new_count->val - old_count->val;
		ena = new_count->ena - old_count->ena;
		run = new_count->run - old_count->run;

		if (ena != run && run != 0)
			val = val * ena / run;
		ret = expr__add_id_val_source_count(pctx, n, val, source_count);
		if (ret)
			return ret;
	}

	for (int i = 0; metric_refs && metric_refs[i].metric_name; i++) {
		int ret = expr__add_ref(pctx, &metric_refs[i]);

		if (ret)
			return ret;
	}

	return 0;
}
