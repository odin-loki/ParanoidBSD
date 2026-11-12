efx_vpd_hunk_reinit(
	__in_bcount(size)	caddr_t data,
	__in			size_t size,
	__in			boolean_t wantpid)
{
	unsigned int offset = 0;
	unsigned int pos;
	efx_byte_t byte;
	uint8_t cksum;
	efx_rc_t rc;

	if (size < 0x100) {
		rc = ENOSPC;
		goto fail1;
	}

	if (wantpid) {
		memcpy(data + offset, __efx_vpd_blank_pid,
		    sizeof (__efx_vpd_blank_pid));
		offset += sizeof (__efx_vpd_blank_pid);
	}

	memcpy(data + offset, __efx_vpd_blank_r, sizeof (__efx_vpd_blank_r));
	offset += sizeof (__efx_vpd_blank_r);

	/* Update checksum */
	cksum = 0;
	for (pos = 0; pos < offset; pos++)
		cksum += data[pos];
	data[offset - 1] -= cksum;

	/* Append trailing tag */
	EFX_POPULATE_BYTE_3(byte,
			    TAG_TYPE, TAG_TYPE_SMALL_ITEM_DECODE,
			    TAG_SMALL_ITEM_NAME, TAG_NAME_END_DECODE,
			    TAG_SMALL_ITEM_SIZE, 0);
	data[offset] = EFX_BYTE_FIELD(byte, EFX_BYTE_0);
	offset++;

	return (0);

fail1:
	EFSYS_PROBE1(fail1, efx_rc_t, rc);

	return (rc);
}
