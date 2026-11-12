static bool map_guarded_stack(void)
{
	int ret;
	uint64_t *buf;
	uint64_t expected_cap;
	int elem;
	bool pass = true;

	buf = (void *)my_syscall3(__NR_map_shadow_stack, 0, page_size,
				  SHADOW_STACK_SET_MARKER |
				  SHADOW_STACK_SET_TOKEN);
	if (buf == MAP_FAILED) {
		ksft_print_msg("Failed to map %lu byte GCS: %d\n",
			       page_size, errno);
		return false;
	}
	ksft_print_msg("Mapped GCS at %p-%p\n", buf,
		       (void *)((uint64_t)buf + page_size));

	/* The top of the newly allocated region should be 0 */
	elem = (page_size / sizeof(uint64_t)) - 1;
	if (buf[elem]) {
		ksft_print_msg("Last entry is 0x%llx not 0x0\n", buf[elem]);
		pass = false;
	}

	/* Then a valid cap token */
	elem--;
	expected_cap = ((uint64_t)buf + page_size - 16);
	expected_cap &= GCS_CAP_ADDR_MASK;
	expected_cap |= GCS_CAP_VALID_TOKEN;
	if (buf[elem] != expected_cap) {
		ksft_print_msg("Cap entry is 0x%llx not 0x%llx\n",
			       buf[elem], expected_cap);
		pass = false;
	}
	ksft_print_msg("cap token is 0x%llx\n", buf[elem]);

	/* The rest should be zeros */
	for (elem = 0; elem < page_size / sizeof(uint64_t) - 2; elem++) {
		if (!buf[elem])
			continue;
		ksft_print_msg("GCS slot %d is 0x%llx not 0x0\n",
			       elem, buf[elem]);
		pass = false;
	}

	ret = munmap(buf, page_size);
	if (ret != 0) {
		ksft_print_msg("Failed to unmap %ld byte GCS: %d\n",
			       page_size, errno);
		pass = false;
	}

	return pass;
}
