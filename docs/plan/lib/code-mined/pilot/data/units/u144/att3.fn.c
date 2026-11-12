static inline bool test_compression_spec(unsigned int mask)
{
	/*@ assert (F2FS_SPEC_compress_algorithm
			| F2FS_SPEC_compress_log_size
			| F2FS_SPEC_compress_extension
			| F2FS_SPEC_nocompress_extension
			| F2FS_SPEC_compress_chksum
			| F2FS_SPEC_compress_mode) == 0x3f; */
	return mask & (F2FS_SPEC_compress_algorithm
			| F2FS_SPEC_compress_log_size
			| F2FS_SPEC_compress_extension
			| F2FS_SPEC_nocompress_extension
			| F2FS_SPEC_compress_chksum
			| F2FS_SPEC_compress_mode);
}
