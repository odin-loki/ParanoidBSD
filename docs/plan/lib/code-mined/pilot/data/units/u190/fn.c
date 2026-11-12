static inline int32_t
ocs_list_on_list(ocs_list_link_t *link)
{
	return (link->next != NULL);
}
