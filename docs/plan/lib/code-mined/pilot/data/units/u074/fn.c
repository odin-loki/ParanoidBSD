static int test_uffd_api(bool use_dev)
{
	struct uffdio_api uffdio_api;
	int uffd;

	uffd_test_start("UFFDIO_API (with %s)",
			use_dev ? "/dev/userfaultfd" : "syscall");

	if (use_dev)
		uffd = uffd_open_dev(UFFD_FLAGS);
	else
		uffd = uffd_open_sys(UFFD_FLAGS);
	if (uffd < 0) {
		uffd_test_skip("cannot open userfaultfd handle");
		return 0;
	}

	/* Test wrong UFFD_API */
	uffdio_api.api = 0xab;
	uffdio_api.features = 0;
	if (ioctl(uffd, UFFDIO_API, &uffdio_api) == 0) {
		uffd_test_fail("UFFDIO_API should fail with wrong api but didn't");
		goto out;
	}

	/* Test wrong feature bit */
	uffdio_api.api = UFFD_API;
	uffdio_api.features = BIT_ULL(63);
	if (ioctl(uffd, UFFDIO_API, &uffdio_api) == 0) {
		uffd_test_fail("UFFDIO_API should fail with wrong feature but didn't");
		goto out;
	}

	/* Test normal UFFDIO_API */
	uffdio_api.api = UFFD_API;
	uffdio_api.features = 0;
	if (ioctl(uffd, UFFDIO_API, &uffdio_api)) {
		uffd_test_fail("UFFDIO_API should succeed but failed");
		goto out;
	}

	/* Test double requests of UFFDIO_API with a random feature set */
	uffdio_api.features = BIT_ULL(0);
	if (ioctl(uffd, UFFDIO_API, &uffdio_api) == 0) {
		uffd_test_fail("UFFDIO_API should reject initialized uffd");
		goto out;
	}

	uffd_test_pass();
out:
	close(uffd);
	/* We have a valid uffd handle */
	return 1;
}
