static int parse_setup_node_list(void)
{
	struct thread_data *td;
	char *str0, *str;
	int t;

	if (!g->p.node_list_str)
		return 0;

	dprintf("g->p.nr_tasks: %d\n", g->p.nr_tasks);

	str0 = str = strdup(g->p.node_list_str);
	t = 0;

	BUG_ON(!str);

	tprintf("# binding tasks to NODEs:\n");
	tprintf("# ");

	while (true) {
		int bind_node, bind_node_0, bind_node_1;
		char *tok, *tok_end, *tok_step, *tok_mul;
		int step;
		int mul;

		tok = strsep(&str, ",");
		if (!tok)
			break;

		tok_end = strstr(tok, "-");

		dprintf("\ntoken: {%s}, end: {%s}\n", tok, tok_end);
		if (!tok_end) {
			/* Single NODE specified: */
			bind_node_0 = bind_node_1 = atol(tok);
		} else {
			/* NODE range specified (for example: "5-11"): */
			bind_node_0 = atol(tok);
			bind_node_1 = atol(tok_end + 1);
		}

		step = 1;
		tok_step = strstr(tok, "#");
		if (tok_step) {
			step = atol(tok_step + 1);
			BUG_ON(step <= 0 || step >= g->p.nr_nodes);
		}

		/* Multiplicator shortcut, "0x8" is a shortcut for: "0,0,0,0,0,0,0,0" */
		mul = 1;
		tok_mul = strstr(tok, "x");
		if (tok_mul) {
			mul = atol(tok_mul + 1);
			BUG_ON(mul <= 0);
		}

		dprintf("NODEs: %d-%d #%d\n", bind_node_0, bind_node_1, step);

		if (bind_node_0 >= g->p.nr_nodes || bind_node_1 >= g->p.nr_nodes) {
			printf("\nTest not applicable, system has only %d nodes.\n", g->p.nr_nodes);
			return -1;
		}

		BUG_ON(bind_node_0 < 0 || bind_node_1 < 0);
		BUG_ON(bind_node_0 > bind_node_1);

		for (bind_node = bind_node_0; bind_node <= bind_node_1; bind_node += step) {
			int i;

			for (i = 0; i < mul; i++) {
				if (t >= g->p.nr_tasks || !node_has_cpus(bind_node)) {
					printf("\n# NOTE: ignoring bind NODEs starting at NODE#%d\n", bind_node);
					goto out;
				}
				td = g->threads + t;

				if (!t)
					tprintf(" %2d", bind_node);
				else
					tprintf(",%2d", bind_node);

				td->bind_node = bind_node;
				t++;
			}
		}
	}
out:

	tprintf("\n");

	if (t < g->p.nr_tasks)
		printf("# NOTE: %d tasks mem-bound, %d tasks unbound\n", t, g->p.nr_tasks - t);

	free(str0);
	return 0;
}
