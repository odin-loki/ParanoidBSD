#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <liblattzfs.h>

int
main(int argc, char *argv[])
{
	lattzfs_zpool_errata_t errata;
	lattzfs_zpool_status_t status;
	lattzfs_ctx_t *ctx;

	if (argc != 2) {
		printf("USAGE: %s poolname\n", argv[0]);
		return (1);
	}

	ctx = lattzfs_ctx_new(argv[1], LATTZFS_FLAG_LAZY_OPEN);
	if (ctx == NULL) {
		printf("ctx null\n");
		return (1);
	}

	status = 0;
	errata = 0;

	if (!lattzfs_zpool_get_status(ctx, &status, &errata)) {
		printf("get_status error\n");
		return (1);
	}

	printf("Status: %d\n", status);
	printf("Errata: 0x%016lx\n", (uint64_t)errata);

	lattzfs_ctx_free(&ctx);

	return (0);
}
