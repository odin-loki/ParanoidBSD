/*
 * oracle.c -- reference implementation for PBSD batch b0209.
 *
 * Mock kernel infrastructure plus ref_* functions from linux_domain.c,
 * linux_folio.c, linux_eventfd.c, and linux_cmdline.c.
 */

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef MAXMEMDOM
#define MAXMEMDOM 16
#endif

#define EBADF 9

#define KASSERT(cond, msg) do { (void)(cond); (void)(msg); } while (0)

#define unlikely(x) (x)

typedef uintptr_t gfp_t;
#define __GFP_COMP 0

struct domainset {
	int ds_kind;	/* 0 = RR, 1 = PREF */
	int ds_node;
};

struct device;
typedef struct device *device_t;

struct device {
	device_t bsddev;
};

struct page {
	unsigned long pg_cookie;
};

struct folio;

struct folio_batch {
	uint8_t nr;
	struct folio *folios[15];
};

struct thread {
	int dummy;
};

struct cap_rights {
	int dummy;
};

struct file {
	int f_id;
};

struct eventfd_ctx {
	int efd_id;
};

struct thread *curthread;
const struct cap_rights cap_no_rights;

int bootverbose;

static struct domainset mock_ds_rr;
static struct domainset mock_ds_pref[MAXMEMDOM];

int mock_bus_get_domain_ret;
int mock_bus_get_domain_val;

struct page *mock_alloc_pages_ret;
gfp_t mock_alloc_pages_last_gfp;
unsigned int mock_alloc_pages_last_order;
int mock_alloc_pages_calls;

#define MOCK_RELEASE_LOG 32
struct folio *mock_release_folios[MOCK_RELEASE_LOG][15];
int mock_release_counts[MOCK_RELEASE_LOG];
int mock_release_log_n;

int mock_fget_ret;
struct file *mock_fget_fp;
int mock_fget_last_fd;
int mock_fget_calls;

struct eventfd_ctx *mock_eventfd_get_ret;
int mock_eventfd_get_last_fp;
int mock_eventfd_get_calls;

int mock_fdrop_calls;
struct file *mock_fdrop_last_fp;

int mock_eventfd_put_calls;
struct eventfd_ctx *mock_eventfd_put_last_ctx;

#define MOCK_ENV_SLOTS 64
char mock_env_names[MOCK_ENV_SLOTS][80];
const char *mock_env_values[MOCK_ENV_SLOTS];
int mock_env_count;

int mock_kern_getenv_calls;
char mock_kern_getenv_last[80];

static inline void *
ERR_PTR(long error)
{
	return ((void *)(intptr_t)error);
}

static inline bool
IS_ERR(const void *ptr)
{
	return ((uintptr_t)ptr >= (uintptr_t)-4095);
}

static inline long
PTR_ERR(const void *ptr)
{
	return ((intptr_t)ptr);
}

static inline unsigned int
folio_batch_count(struct folio_batch *fbatch)
{
	return (fbatch->nr);
}

static inline void
folio_batch_reinit(struct folio_batch *fbatch)
{
	fbatch->nr = 0;
}

struct domainset *
DOMAINSET_RR(void)
{
	mock_ds_rr.ds_kind = 0;
	mock_ds_rr.ds_node = -1;
	return (&mock_ds_rr);
}

struct domainset *
DOMAINSET_PREF(int node)
{
	mock_ds_pref[node].ds_kind = 1;
	mock_ds_pref[node].ds_node = node;
	return (&mock_ds_pref[node]);
}

int
bus_get_domain(device_t dev, int *domain)
{
	(void)dev;
	if (domain != NULL)
		*domain = mock_bus_get_domain_val;
	return (mock_bus_get_domain_ret);
}

struct page *
alloc_pages(gfp_t gfp, unsigned int order)
{
	mock_alloc_pages_calls++;
	mock_alloc_pages_last_gfp = gfp;
	mock_alloc_pages_last_order = order;
	return (mock_alloc_pages_ret);
}

void
release_pages(struct folio **folios, unsigned int count)
{
	int slot;

	if (mock_release_log_n < MOCK_RELEASE_LOG) {
		slot = mock_release_log_n++;
		mock_release_counts[slot] = (int)count;
		if (count > 15)
			count = 15;
		for (unsigned int i = 0; i < count; i++)
			mock_release_folios[slot][i] = folios[i];
	}
}

int
fget_unlocked(struct thread *td, int fd, const struct cap_rights *rights,
    struct file **fpp)
{
	(void)td;
	(void)rights;
	mock_fget_calls++;
	mock_fget_last_fd = fd;
	if (mock_fget_ret != 0)
		return (mock_fget_ret);
	if (fpp != NULL)
		*fpp = mock_fget_fp;
	return (0);
}

struct eventfd_ctx *
eventfd_get(struct file *fp)
{
	mock_eventfd_get_calls++;
	mock_eventfd_get_last_fp = fp != NULL ? fp->f_id : -1;
	return (mock_eventfd_get_ret);
}

void
fdrop(struct file *fp, struct thread *td)
{
	(void)td;
	mock_fdrop_calls++;
	mock_fdrop_last_fp = fp;
}

void
eventfd_put(struct eventfd_ctx *ctx)
{
	mock_eventfd_put_calls++;
	mock_eventfd_put_last_ctx = ctx;
}

const char *
kern_getenv(const char *name)
{
	int i;

	mock_kern_getenv_calls++;
	if (name != NULL) {
		strncpy(mock_kern_getenv_last, name, sizeof(mock_kern_getenv_last) - 1);
		mock_kern_getenv_last[sizeof(mock_kern_getenv_last) - 1] = '\0';
	} else
		mock_kern_getenv_last[0] = '\0';
	for (i = 0; i < mock_env_count; i++) {
		if (strcmp(mock_env_names[i], name) == 0)
			return (mock_env_values[i]);
	}
	return (NULL);
}

void
mock_reset_b0209(void)
{
	int i;

	mock_bus_get_domain_ret = 0;
	mock_bus_get_domain_val = 0;
	mock_alloc_pages_ret = NULL;
	mock_alloc_pages_last_gfp = 0;
	mock_alloc_pages_last_order = 0;
	mock_alloc_pages_calls = 0;
	mock_release_log_n = 0;
	memset(mock_release_counts, 0, sizeof(mock_release_counts));
	memset(mock_release_folios, 0, sizeof(mock_release_folios));
	mock_fget_ret = 0;
	mock_fget_fp = NULL;
	mock_fget_last_fd = 0;
	mock_fget_calls = 0;
	mock_eventfd_get_ret = NULL;
	mock_eventfd_get_last_fp = 0;
	mock_eventfd_get_calls = 0;
	mock_fdrop_calls = 0;
	mock_fdrop_last_fp = NULL;
	mock_eventfd_put_calls = 0;
	mock_eventfd_put_last_ctx = NULL;
	mock_env_count = 0;
	mock_kern_getenv_calls = 0;
	mock_kern_getenv_last[0] = '\0';
	bootverbose = 0;
	curthread = NULL;
	memset(&mock_ds_rr, 0, sizeof(mock_ds_rr));
	memset(mock_ds_pref, 0, sizeof(mock_ds_pref));
	for (i = 0; i < MOCK_ENV_SLOTS; i++) {
		mock_env_names[i][0] = '\0';
		mock_env_values[i] = NULL;
	}
}

void
mock_set_env(const char *name, const char *value)
{
	int i;

	if (name == NULL)
		return;
	for (i = 0; i < mock_env_count; i++) {
		if (strcmp(mock_env_names[i], name) == 0) {
			mock_env_values[i] = value;
			return;
		}
	}
	if (mock_env_count >= MOCK_ENV_SLOTS)
		return;
	strncpy(mock_env_names[mock_env_count], name,
	    sizeof(mock_env_names[0]) - 1);
	mock_env_names[mock_env_count][sizeof(mock_env_names[0]) - 1] = '\0';
	mock_env_values[mock_env_count] = value;
	mock_env_count++;
}

/* ------------------------------------------------------------------------ */
/* hbsd/src/sys/compat/linuxkpi/common/src/linux_domain.c                   */
/* ------------------------------------------------------------------------ */

/*-
 * Copyright (c) 2021 NVIDIA Networking
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

struct domainset *
ref_linux_get_vm_domain_set(int node)
{
	KASSERT(node < MAXMEMDOM, ("Invalid VM domain %d", node));

	if (node < 0)
		return (DOMAINSET_RR());
	else
		return (DOMAINSET_PREF(node));
}

int
ref_linux_dev_to_node(struct device *dev)
{
	int numa_domain;

	if (dev == NULL || dev->bsddev == NULL ||
	    bus_get_domain(dev->bsddev, &numa_domain) != 0)
		return (-1);
	else
		return (numa_domain);
}

/* ------------------------------------------------------------------------ */
/* hbsd/src/sys/compat/linuxkpi/common/src/linux_folio.c                    */
/* ------------------------------------------------------------------------ */

/*-
 * Copyright (c) 2024-2025 The FreeBSD Foundation
 * Copyright (c) 2024-2025 Jean-Sébastien Pédron
 *
 * This software was developed by Jean-Sébastien Pédron under sponsorship
 * from the FreeBSD Foundation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

struct folio *
ref_folio_alloc(gfp_t gfp, unsigned int order)
{
	struct page *page;
	struct folio *folio;

	/*
	 * Allocated pages are wired already. There is no need to increase a
	 * refcount here.
	 */
	page = alloc_pages(gfp | __GFP_COMP, order);
	folio = (struct folio *)page;

	return (folio);
}

void
ref___folio_batch_release(struct folio_batch *fbatch)
{
	release_pages(fbatch->folios, folio_batch_count(fbatch));

	folio_batch_reinit(fbatch);
}

/* ------------------------------------------------------------------------ */
/* hbsd/src/sys/compat/linuxkpi/common/src/linux_eventfd.c                    */
/* ------------------------------------------------------------------------ */

/*-
 * Copyright (c) 2025 The FreeBSD Foundation
 * Copyright (c) 2025 Jean-Sébastien Pédron
 *
 * This software was developed by Jean-Sébastien Pédron under sponsorship
 * from the FreeBSD Foundation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

struct eventfd_ctx *
ref_lkpi_eventfd_ctx_fdget(int fd)
{
	struct file *fp;
	struct eventfd_ctx *ctx;

	/* Lookup file pointer by file descriptor index. */
	if (fget_unlocked(curthread, fd, &cap_no_rights, &fp) != 0)
		return (ERR_PTR(-EBADF));

	/*
	 * eventfd_get() bumps the refcount, so we can safely release the
	 * reference on the file itself afterwards.
	 */
	ctx = eventfd_get(fp);
	fdrop(fp, curthread);

	if (ctx == NULL)
		return (ERR_PTR(-EBADF));

	return (ctx);
}

void
ref_lkpi_eventfd_ctx_put(struct eventfd_ctx *ctx)
{
	eventfd_put(ctx);
}

/* ------------------------------------------------------------------------ */
/* hbsd/src/sys/compat/linuxkpi/common/src/linux_cmdline.c                    */
/* ------------------------------------------------------------------------ */

/*-
 * Copyright (c) 2022 Beckhoff Automation GmbH & Co. KG
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

const char *
ref_video_get_options(const char *connector_name)
{
	char tunable[64];
	const char *options;

	/*
	 * A user may use loader tunables to set a specific mode for the
	 * console. Tunables are read in the following order:
	 *     1. kern.vt.fb.modes.$connector_name
	 *     2. kern.vt.fb.default_mode
	 *
	 * Example of a mode specific to the LVDS connector:
	 *     kern.vt.fb.modes.LVDS="1024x768"
	 *
	 * Example of a mode applied to all connectors not having a
	 * connector-specific mode:
	 *     kern.vt.fb.default_mode="640x480"
	 */
	snprintf(tunable, sizeof(tunable), "kern.vt.fb.modes.%s",
	    connector_name);
	if (bootverbose) {
		printf("[drm] Connector %s: get mode from tunables:\n", connector_name);
		printf("[drm]  - %s\n", tunable);
		printf("[drm]  - kern.vt.fb.default_mode\n");
	}
	options = kern_getenv(tunable);
	if (options == NULL)
		options = kern_getenv("kern.vt.fb.default_mode");

	return (options);
}
