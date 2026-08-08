/*
 * PBSD batch b0209 -- reference oracle.
 *
 * The four HardenedBSD sources of this batch, concatenated, with every
 * function renamed with a "ref_" prefix.  Function bodies are UNMODIFIED.
 *
 * The kernel headers the bodies include are not available to a userspace
 * differential test, so the declarations, types and defines those headers
 * would supply are provided below.  The harness defines every extern.
 *
 *	hbsd/src/sys/compat/linuxkpi/common/src/linux_domain.c
 *	hbsd/src/sys/compat/linuxkpi/common/src/linux_folio.c
 *	hbsd/src/sys/compat/linuxkpi/common/src/linux_eventfd.c
 *	hbsd/src/sys/compat/linuxkpi/common/src/linux_cmdline.c
 */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <errno.h>

/* sys/systm.h: kernel printf, routed to the harness so it is observable. */
extern int lkpi_printf(const char *fmt, ...);
#define	printf		lkpi_printf

/* sys/systm.h: KASSERT is a no-op without INVARIANTS. */
#define	KASSERT(exp, msg)	((void)0)

/* sys/domainset.h */
#define	MAXMEMDOM	8
struct domainset {
	int	ds_id;
};
extern struct domainset domainset_roundrobin;
extern struct domainset domainset_prefer[MAXMEMDOM];
#define	DOMAINSET_RR()		(&domainset_roundrobin)
#define	DOMAINSET_PREF(domain)	(&domainset_prefer[(domain)])

/* sys/bus.h, linux/device.h */
struct pbsd_bsddev;
typedef struct pbsd_bsddev *device_t;
struct device {
	device_t	bsddev;
};
extern int bus_get_domain(device_t dev, int *domain);

/* linux/gfp.h, linux/mm.h, linux/page.h */
typedef unsigned int gfp_t;
#define	__GFP_COMP	0x4000u
struct page;
struct folio;
extern struct page *alloc_pages(gfp_t gfp, unsigned int order);
extern void release_pages(struct folio **folios, int nr);

/* linux/pagevec.h */
#define	PAGEVEC_SIZE	15
struct folio_batch {
	uint8_t		nr;
	struct folio	*folios[PAGEVEC_SIZE];
};

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

/* sys/file.h, sys/filedesc.h, linux/eventfd.h, linux/err.h */
struct thread;
struct file;
struct eventfd_ctx;
struct cap_rights {
	int	cr_dummy;
};
extern struct thread *lkpi_curthread;
#define	curthread	lkpi_curthread
extern struct cap_rights cap_no_rights;
extern int fget_unlocked(struct thread *td, int fd,
    const struct cap_rights *rights, struct file **fpp);
extern void fdrop(struct file *fp, struct thread *td);
extern struct eventfd_ctx *eventfd_get(struct file *fp);
extern void eventfd_put(struct eventfd_ctx *ctx);
#define	ERR_PTR(error)	((void *)(intptr_t)(error))

/* sys/systm.h, video/cmdline.h */
extern int bootverbose;
extern char *kern_getenv(const char *name);

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
