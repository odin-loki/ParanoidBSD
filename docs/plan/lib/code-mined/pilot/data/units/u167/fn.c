STORAGE_CLASS_DEBUG_C bool is_debug_buffer_empty(void)
{
	return (debug_data_ptr->head == debug_data_ptr->tail);
}
