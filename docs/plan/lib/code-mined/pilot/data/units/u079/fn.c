static int wpabuf_tests(void)
{
	int errors = 0;
	void *ptr;
	struct wpabuf *buf;

	wpa_printf(MSG_INFO, "wpabuf tests");

	ptr = os_malloc(100);
	if (ptr) {
		buf = wpabuf_alloc_ext_data(ptr, 100);
		if (buf) {
			if (wpabuf_resize(&buf, 100) < 0)
				errors++;
			else
				wpabuf_put(buf, 100);
			wpabuf_free(buf);
		} else {
			errors++;
			os_free(ptr);
		}
	} else {
		errors++;
	}

	buf = wpabuf_alloc(100);
	if (buf) {
		struct wpabuf *buf2;

		wpabuf_put(buf, 100);
		if (wpabuf_resize(&buf, 100) < 0)
			errors++;
		else
			wpabuf_put(buf, 100);
		buf2 = wpabuf_concat(buf, NULL);
		if (buf2 != buf)
			errors++;
		wpabuf_free(buf2);
	} else {
		errors++;
	}

	buf = NULL;
	buf = wpabuf_zeropad(buf, 10);
	if (buf != NULL)
		errors++;

	if (errors) {
		wpa_printf(MSG_ERROR, "%d wpabuf test(s) failed", errors);
		return -1;
	}

	return 0;
}
