int
archive_entry_sparse_reset(struct archive_entry * entry)
{
	entry->sparse_p = entry->sparse_head;

	return archive_entry_sparse_count(entry);
}
