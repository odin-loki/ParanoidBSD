static size_t
sja1110_vl_forwarding_params_entry_packing(void *buf, void *entry_ptr,
					   enum packing_op op)
{
	struct sja1105_vl_forwarding_params_entry *entry = entry_ptr;
	const size_t size = SJA1105_SIZE_VL_FORWARDING_PARAMS_ENTRY;
	int offset, i;

	for (i = 0, offset = 8; i < 8; i++, offset += 11)
		sja1105_packing(buf, &entry->partspc[i],
				offset + 10, offset + 0, size, op);
	sja1105_packing(buf, &entry->debugen, 7, 7, size, op);
	return size;
}
