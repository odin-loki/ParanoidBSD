/*-
 * Copyright (c) 2015 The FreeBSD Foundation
 *
 * This software was developed by Konstantin Belousov
 * under sponsorship from the FreeBSD Foundation.
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

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/queue.h>
#include "namespace.h"
#include <stdlib.h>
#include "un-namespace.h"

#include "thr_private.h"

struct psh {
	LIST_ENTRY(psh) link;
	void *key;
	void *val;
};

LIST_HEAD(pshared_hash_head, psh);
#define	HASH_SIZE	128
static struct pshared_hash_head pshared_hash[HASH_SIZE];
#define	PSHARED_KEY_HASH(key)	(((unsigned long)(key) >> 8) % HASH_SIZE)
/* XXXKIB: lock could be split to per-hash chain, if appears contested */
static struct urwlock pshared_lock = DEFAULT_URWLOCK;
static int page_size;

void
ref___thr_pshared_init(void)
{
	int i;

	page_size = getpagesize();
	THR_ASSERT(page_size >= THR_PAGE_SIZE_MIN,
	    "THR_PAGE_SIZE_MIN is too large");

	_thr_urwlock_init(&pshared_lock);
	for (i = 0; i < HASH_SIZE; i++)
		LIST_INIT(&pshared_hash[i]);
}

static void
ref_pshared_rlock(struct pthread *curthread)
{

	curthread->locklevel++;
	_thr_rwl_rdlock(&pshared_lock);
}

static void
ref_pshared_wlock(struct pthread *curthread)
{

	curthread->locklevel++;
	_thr_rwl_wrlock(&pshared_lock);
}

static void
ref_pshared_unlock(struct pthread *curthread)
{

	_thr_rwl_unlock(&pshared_lock);
	curthread->locklevel--;
	_thr_ast(curthread);
}

/*
 * Among all processes sharing a lock only one executes
 * pthread_lock_destroy().  Other processes still have the hash and
 * mapped off-page.
 *
 * Mitigate the problem by checking the liveness of all hashed keys
 * periodically.  Right now this is executed on each
 * pthread_lock_destroy(), but may be done less often if found to be
 * too time-consuming.
 */
static void
ref_pshared_gc(struct pthread *curthread)
{
	struct pshared_hash_head *hd;
	struct psh *h, *h1;
	int error, i;

	ref_pshared_wlock(curthread);
	for (i = 0; i < HASH_SIZE; i++) {
		hd = &pshared_hash[i];
		LIST_FOREACH_SAFE(h, hd, link, h1) {
			error = _umtx_op(NULL, UMTX_OP_SHM, UMTX_SHM_ALIVE,
			    h->val, NULL);
			if (error == 0)
				continue;
			LIST_REMOVE(h, link);
			munmap(h->val, page_size);
			free(h);
		}
	}
	ref_pshared_unlock(curthread);
}

static void *
ref_pshared_lookup(void *key)
{
	struct pshared_hash_head *hd;
	struct psh *h;

	hd = &pshared_hash[PSHARED_KEY_HASH(key)];
	LIST_FOREACH(h, hd, link) {
		if (h->key == key)
			return (h->val);
	}
	return (NULL);
}

static int
ref_pshared_insert(void *key, void **val)
{
	struct pshared_hash_head *hd;
	struct psh *h;

	hd = &pshared_hash[PSHARED_KEY_HASH(key)];
	LIST_FOREACH(h, hd, link) {
		/*
		 * When the key already exists in the hash, we should
		 * return either the new (just mapped) or old (hashed)
		 * val, and the other val should be unmapped to avoid
		 * address space leak.
		 *
		 * If two threads perform lock of the same object
		 * which is not yet stored in the pshared_hash, then
		 * the val already inserted by the first thread should
		 * be returned, and the second val freed (order is by
		 * the pshared_lock()).  Otherwise, if we unmap the
		 * value obtained from the hash, the first thread
		 * might operate on an unmapped off-page object.
		 *
		 * There is still an issue: if hashed key was unmapped
		 * and then other page is mapped at the same key
		 * address, the hash would return the old val.  I
		 * decided to handle the race of simultaneous hash
		 * insertion, leaving the unlikely remap problem
		 * unaddressed.
		 */
		if (h->key == key) {
			if (h->val != *val) {
				munmap(*val, page_size);
				*val = h->val;
			}
			return (1);
		}
	}

	h = malloc(sizeof(*h));
	if (h == NULL)
		return (0);
	h->key = key;
	h->val = *val;
	LIST_INSERT_HEAD(hd, h, link);
	return (1);
}

static void *
ref_pshared_remove(void *key)
{
	struct pshared_hash_head *hd;
	struct psh *h;
	void *val;

	hd = &pshared_hash[PSHARED_KEY_HASH(key)];
	LIST_FOREACH(h, hd, link) {
		if (h->key == key) {
			LIST_REMOVE(h, link);
			val = h->val;
			free(h);
			return (val);
		}
	}
	return (NULL);
}

static void
ref_pshared_clean(void *key, void *val)
{

	if (val != NULL)
		munmap(val, page_size);
	_umtx_op(NULL, UMTX_OP_SHM, UMTX_SHM_DESTROY, key, NULL);
}

static void
ref_pshared_destroy(struct pthread *curthread, void *key)
{
	void *val;

	ref_pshared_wlock(curthread);
	val = ref_pshared_remove(key);
	ref_pshared_unlock(curthread);
	ref_pshared_clean(key, val);
}

void *
ref___thr_pshared_offpage(void *key, int doalloc)
{
	struct pthread *curthread;
	void *res;
	int fd, ins_done;

	curthread = _get_curthread();
	if (doalloc) {
		ref_pshared_destroy(curthread, key);
		res = NULL;
	} else {
		ref_pshared_rlock(curthread);
		res = ref_pshared_lookup(key);
		ref_pshared_unlock(curthread);
		if (res != NULL)
			return (res);
	}
	fd = _umtx_op(NULL, UMTX_OP_SHM, doalloc ? UMTX_SHM_CREAT :
	    UMTX_SHM_LOOKUP, key, NULL);
	if (fd == -1)
		return (NULL);
	res = mmap(NULL, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	if (res == MAP_FAILED)
		return (NULL);
	ref_pshared_wlock(curthread);
	ins_done = ref_pshared_insert(key, &res);
	ref_pshared_unlock(curthread);
	if (!ins_done) {
		ref_pshared_clean(key, res);
		res = NULL;
	}
	return (res);
}

void
ref___thr_pshared_destroy(void *key)
{
	struct pthread *curthread;

	curthread = _get_curthread();
	ref_pshared_destroy(curthread, key);
	ref_pshared_gc(curthread);
}

void
ref___thr_pshared_atfork_pre(void)
{

	_thr_rwl_rdlock(&pshared_lock);
}

void
ref___thr_pshared_atfork_post(void)
{

	_thr_rwl_unlock(&pshared_lock);
}
