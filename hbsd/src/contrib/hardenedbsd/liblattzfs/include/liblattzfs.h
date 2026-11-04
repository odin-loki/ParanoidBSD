#ifndef _LIBLATTZFS_H
#define _LIBLATTZFS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#define LATTZFS_VERSION	1

#define LATTZFS_FLAG_LAZY_OPEN	0x1

/*
 * Keep this in-sync with OpenZFS' zpool_errata_t
 *
 * Current version: HardenedBSD 14-CURRENT, 14 Aug 2021
 */
typedef enum _lattzfs_zpool_errata {
	LATT_ZPOOL_ERRATA_NONE=0,
	LATT_ZPOOL_ERRATA_ZOL_2094_SCRUB=1,
	LATT_ZPOOL_ERRATA_ZOL_2094_ASYNC_DESTROY=2,
	LATT_ZPOOL_ERRATA_ZOL_6845_ENCRYPTION=3,
	LATT_ZPOOL_ERRATA_ZOL_8308_ENCRYPTION=4,
} lattzfs_zpool_errata_t;

/*
 * Keep this in-sync with OpenZFS zpool_status_t
 *
 * Current version: HardenedBSD 14-CURRENT, 14 Aug 2021
 */

typedef enum _lattzfs_zpool_status {
	LATT_ZPOOL_STATUS_CORRUPT_CACHE=0,
	LATT_ZPOOL_STATUS_MISSING_DEV_R=1,
	LATT_ZPOOL_STATUS_MISSING_DEV_NR=2,
	LATT_ZPOOL_STATUS_CORRUPT_LABEL_R=3,
	LATT_ZPOOL_STATUS_CORRUPT_LABEL_NR=4,
	LATT_ZPOOL_STATUS_BAD_GUID_SUM=5,
	LATT_ZPOOL_STATUS_CORRUPT_POOL=6,
	LATT_ZPOOL_STATUS_CORRUPT_DATA=7,
	LATT_ZPOOL_STATUS_FAILING_DEV=8,
	LATT_ZPOOL_STATUS_VERSION_NEWER=9,
	LATT_ZPOOL_STATUS_HOSTID_MISMATCH=10,
	LATT_ZPOOL_STATUS_HOSTID_ACTIVE=11,
	LATT_ZPOOL_STATUS_HOSTID_REQUIRED=12,
	LATT_ZPOOL_STATUS_IO_FAILURE_WAIT=13,
	LATT_ZPOOL_STATUS_IO_FAILURE_CONTINUE=14,
	LATT_ZPOOL_STATUS_IO_FAILURE_MMP=15,
	LATT_ZPOOL_STATUS_BAD_LOG=16,
	LATT_ZPOOL_STATUS_ERRATA=17,
	LATT_ZPOOL_STATUS_UNSUP_FEAT_READ=18,
	LATT_ZPOOL_STATUS_UNSUP_FEAT_WRITE=19,
	LATT_ZPOOL_STATUS_FAULTED_DEV_R=20,
	LATT_ZPOOL_STATUS_FAULTED_DEV_NR=21,
	LATT_ZPOOL_STATUS_VERSION_OLDER=22,
	LATT_ZPOOL_STATUS_FEAT_DISABLED=23,
	LATT_ZPOOL_STATUS_RESILVERING=24,
	LATT_ZPOOL_STATUS_OFFLINE_DEV=25,
	LATT_ZPOOL_STATUS_REMOVED_DEV=26,
	LATT_ZPOOL_STATUS_REBUILDING=27,
	LATT_ZPOOL_STATUS_REBUILD_SCRUB=28,
	LATT_ZPOOL_STATUS_NON_NATIVE_ASHIFT=29,
	LATT_ZPOOL_STATUS_COMPATIBILITY_ERR=30,
	LATT_ZPOOL_STATUS_INCOMPATIBLE_FEAT=31,
	LATT_ZPOOL_STATUS_OK=32,
} lattzfs_zpool_status_t;

typedef struct _lattzfs_ctx {
	uint64_t	 lc_version;
	uint64_t	 lc_flags;
	void		*lc_zfs_handle;
	void		*lc_pool_handle;
	char		*lc_pool_name;
} lattzfs_ctx_t;

lattzfs_ctx_t *lattzfs_ctx_new(const char *, uint64_t);
void lattzfs_ctx_free(lattzfs_ctx_t **);
bool lattzfs_zpool_get_status(lattzfs_ctx_t *, lattzfs_zpool_status_t *,
    lattzfs_zpool_errata_t *);

uint64_t lattzfs_get_version(lattzfs_ctx_t *);
uint64_t lattzfs_get_flags(lattzfs_ctx_t *);
uint64_t lattzfs_set_flags(lattzfs_ctx_t *, uint64_t);
uint64_t lattzfs_set_flag(lattzfs_ctx_t *, uint64_t);
void *lattzfs_get_zfs_handle(lattzfs_ctx_t *);
void *lattzfs_get_pool_handle(lattzfs_ctx_t *);
char *lattzfs_get_pool_name(lattzfs_ctx_t *);

#ifdef __cplusplus
}
#endif

#endif /* !_LIBLATTZFS_H */
