int
hhook_add_hook_lookup(const struct hookinfo *hki, uint32_t flags)
{
	struct hhook_head **heads_to_hook, *hhh;
	int error, i, n_heads_to_hook;

tryagain:
	error = i = 0;
	/*
	 * Accessing n_hhookheads without hhook_head_list_lock held opens up a
	 * race with hhook_head_register() which we are unlikely to lose, but
	 * nonetheless have to cope with - hence the complex goto logic.
	 */
	n_heads_to_hook = n_hhookheads;
	heads_to_hook = malloc(n_heads_to_hook * sizeof(struct hhook_head *),
	    M_HHOOK, flags & HHOOK_WAITOK ? M_WAITOK : M_NOWAIT);
	if (heads_to_hook == NULL)
		return (ENOMEM);

	HHHLIST_LOCK();
	LIST_FOREACH(hhh, &hhook_head_list, hhh_next) {
		if (hhh->hhh_type == hki->hook_type &&
		    hhh->hhh_id == hki->hook_id) {
			if (i < n_heads_to_hook) {
				heads_to_hook[i] = hhh;
				refcount_acquire(&heads_to_hook[i]->hhh_refcount);
				i++;
			} else {
				/*
				 * We raced with hhook_head_register() which
				 * inserted a hhook_head that we need to hook
				 * but did not malloc space for. Abort this run
				 * and try again.
				 */
				for (i--; i >= 0; i--)
					refcount_release(&heads_to_hook[i]->hhh_refcount);
				free(heads_to_hook, M_HHOOK);
				HHHLIST_UNLOCK();
				goto tryagain;
			}
		}
	}
	HHHLIST_UNLOCK();

	for (i--; i >= 0; i--) {
		if (!error)
			error = hhook_add_hook(heads_to_hook[i], hki, flags);
		refcount_release(&heads_to_hook[i]->hhh_refcount);
	}

	free(heads_to_hook, M_HHOOK);

	return (error);
}
