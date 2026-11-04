/*-
 * CAM request queue management functions.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 1997 Justin T. Gibbs.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification, immediately at the beginning of the file.
 * 2. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/types.h>
#include <sys/malloc.h>
#include <sys/kernel.h>
#include <sys/limits.h>

#include <cam/cam.h>
#include <cam/cam_ccb.h>
#include <cam/cam_queue.h>
#include <cam/cam_debug.h>

static MALLOC_DEFINE(M_CAMQ, "CAM queue", "CAM queue buffers");
static MALLOC_DEFINE(M_CAMDEVQ, "CAM dev queue", "CAM dev queue buffers");
static MALLOC_DEFINE(M_CAMCCBQ, "CAM ccb queue", "CAM ccb queue buffers");

static void	heap_up(cam_pinfo **queue_array, int new_index);
static void	heap_down(cam_pinfo **queue_array, int index,
			  int last_index);

int
camq_init(struct camq *camq, int size)
{
	/*
	 * PBSD: `size + 1` below is undefined at INT_MAX, and a negative
	 * size sets array_size negative. The negative case already fails
	 * - (size + 1) * sizeof() converts to a huge size_t and the
	 * M_NOWAIT malloc cannot satisfy it - so rejecting it up front
	 * changes nothing except that it now says so. INT_MAX is the one
	 * that is UB rather than merely wrong.
	 *
	 * Not reachable today: every caller passes a bus's device count
	 * or an opening count that cam_ccbq_init() and cam_ccbq_resize()
	 * now clamp. camq_init() is exported in cam_queue.h, which is
	 * the same reason those two carry the clamp rather than their
	 * one reachable caller.
	 */
	if (size < 0 || size == INT_MAX)
		return (1);
	bzero(camq, sizeof(*camq));
	camq->array_size = size;
	if (camq->array_size != 0) {
		/*
		 * Heap algorithms like everything numbered from 1, so
		 * allocate one more to account for 0 base.
		 */
		camq->queue_array = malloc((size + 1) * sizeof(cam_pinfo*),
		    M_CAMQ, M_NOWAIT);
		if (camq->queue_array == NULL) {
			printf("camq_init: - cannot malloc array!\n");
			return (1);
		}
	}
	return (0);
}

/*
 * Free a camq structure.  This should only be called if a controller
 * driver fails somehow during its attach routine or is unloaded and has
 * obtained a camq structure.  The XPT should ensure that the queue
 * is empty before calling this routine.
 */
void
camq_fini(struct camq *queue)
{
	if (queue->queue_array != NULL) {
		free(queue->queue_array, M_CAMQ);
	}
}

uint32_t
camq_resize(struct camq *queue, int new_size)
{
	cam_pinfo **new_array;

	KASSERT(new_size >= queue->entries, ("camq_resize: "
	    "New queue size can't accommodate queued entries (%d < %d).",
	    new_size, queue->entries));
	new_array = malloc((new_size + 1) * sizeof(cam_pinfo *), M_CAMQ,
	    M_NOWAIT);
	if (new_array == NULL) {
		/* Couldn't satisfy request */
		return (CAM_RESRC_UNAVAIL);
	}
	/*
	 * Heap algorithms like everything numbered from 1, so
	 * remember that our pointer into the heap array is offset
	 * by one element.
	 */
	if (queue->queue_array != NULL) {
		bcopy(queue->queue_array, new_array,
		    (queue->entries + 1) * sizeof(cam_pinfo *));
		free(queue->queue_array, M_CAMQ);
	}
	queue->queue_array = new_array;
	queue->array_size = new_size;
	return (CAM_REQ_CMP);
}

/*
 * camq_insert: Given an array of cam_pinfo* elememnts with
 * the Heap(1, num_elements) property and array_size - num_elements >= 1,
 * output Heap(1, num_elements+1) including new_entry in the array.
 */
void
camq_insert(struct camq *queue, cam_pinfo *new_entry)
{

	KASSERT(queue->entries < queue->array_size,
	    ("camq_insert: Attempt to insert into a full queue (%d >= %d)",
	    queue->entries, queue->array_size));
	queue->entries++;
	queue->queue_array[queue->entries] = new_entry;
	new_entry->index = queue->entries;
	if (queue->entries != 0)
		heap_up(queue->queue_array, queue->entries);
}

/*
 * camq_remove:  Given an array of cam_pinfo* elevements with the
 * Heap(1, num_elements) property and an index such that 1 <= index <=
 * num_elements, remove that entry and restore the Heap(1, num_elements-1)
 * property.
 */
cam_pinfo *
camq_remove(struct camq *queue, int index)
{
	cam_pinfo *removed_entry;

	if (index <= 0 || index > queue->entries)
		panic("%s: Attempt to remove out-of-bounds index %d "
		    "from queue %p of size %d", __func__, index, queue,
		    queue->entries);

	removed_entry = queue->queue_array[index];
	if (queue->entries != index) {
		queue->queue_array[index] = queue->queue_array[queue->entries];
		queue->queue_array[index]->index = index;
		heap_down(queue->queue_array, index, queue->entries - 1);
	}
	removed_entry->index = CAM_UNQUEUED_INDEX;
	queue->entries--;
	return (removed_entry);
}

/*
 * camq_change_priority:  Given an array of cam_pinfo* elements with the
 * Heap(1, num_entries) property, an index such that 1 <= index <= num_elements,
 * and a new priority for the element at index, change the priority of
 * element index and restore the Heap(0, num_elements) property.
 */
void
camq_change_priority(struct camq *queue, int index, uint32_t new_priority)
{
	if (new_priority > queue->queue_array[index]->priority) {
		queue->queue_array[index]->priority = new_priority;
		heap_down(queue->queue_array, index, queue->entries);
	} else {
		/* new_priority <= old_priority */
		queue->queue_array[index]->priority = new_priority;
		heap_up(queue->queue_array, index);
	}
}

struct cam_devq *
cam_devq_alloc(int devices, int openings)
{
	struct cam_devq *devq;

	devq = (struct cam_devq *)malloc(sizeof(*devq), M_CAMDEVQ, M_NOWAIT);
	if (devq == NULL) {
		printf("cam_devq_alloc: - cannot malloc!\n");
		return (NULL);
	}
	if (cam_devq_init(devq, devices, openings) != 0) {
		free(devq, M_CAMDEVQ);
		return (NULL);
	}
	return (devq);
}

int
cam_devq_init(struct cam_devq *devq, int devices, int openings)
{

	bzero(devq, sizeof(*devq));
	mtx_init(&devq->send_mtx, "CAM queue lock", NULL, MTX_DEF);
	if (camq_init(&devq->send_queue, devices) != 0)
		return (1);
	devq->send_openings = openings;
	devq->send_active = 0;
	return (0);
}

void
cam_devq_free(struct cam_devq *devq)
{

	camq_fini(&devq->send_queue);
	mtx_destroy(&devq->send_mtx);
	free(devq, M_CAMDEVQ);
}

uint32_t
cam_devq_resize(struct cam_devq *camq, int devices)
{
	uint32_t retval;

	retval = camq_resize(&camq->send_queue, devices);
	return (retval);
}

struct cam_ccbq *
cam_ccbq_alloc(int openings)
{
	struct cam_ccbq *ccbq;

	ccbq = (struct cam_ccbq *)malloc(sizeof(*ccbq), M_CAMCCBQ, M_NOWAIT);
	if (ccbq == NULL) {
		printf("cam_ccbq_alloc: - cannot malloc!\n");
		return (NULL);
	}
	if (cam_ccbq_init(ccbq, openings) != 0) {
		free(ccbq, M_CAMCCBQ);
		return (NULL);		
	}

	return (ccbq);
}

void
cam_ccbq_free(struct cam_ccbq *ccbq)
{
	if (ccbq) {
		cam_ccbq_fini(ccbq);
		free(ccbq, M_CAMCCBQ);
	}
}

uint32_t
cam_ccbq_resize(struct cam_ccbq *ccbq, int new_size)
{
	int delta;

	/*
	 * PBSD: clamp before anything is computed from new_size.
	 *
	 * XPT_REL_SIMQ with RELSIM_ADJUST_OPENINGS reaches here with
	 * crs->openings copied verbatim out of a userland ccb -
	 * xpt_merge_ccb() bcopy()s the whole union body, and pass(4)
	 * does not restrict this function code. cam_xpt.c:2939 checks
	 * `crs->openings > 0` and says why: "Don't ever go below one
	 * opening". That is the lower bound, and there was no upper one.
	 *
	 * With new_size = INT_MAX:
	 *
	 *   new_size + new_size / 2   3221225470, wraps to -1073741826
	 *   fls(-1073741826)          32
	 *   1 << 32                   a shift by the width of the type
	 *
	 * two counts of undefined behaviour, and `total_openings +=
	 * delta` overflows as well. INT_MIN underflows the same sum from
	 * the other side, which is why both bounds are here and not just
	 * the one the reachable caller needs. On a target where the wrap is
	 * benign the result is imax(64, 1) == 64, so the queue is NOT
	 * resized while dev_openings has been raised to ~2 billion.
	 *
	 * Not a memory-safety bug: cam_ccbq_insert_ccb() resizes on a
	 * full queue and spills the lowest-priority ccb to
	 * queue_extra_head when that fails, so camq_insert()'s KASSERT
	 * is an invariant its caller maintains rather than the only
	 * check. It is the arithmetic that is wrong.
	 */
	if (new_size < 0)
		new_size = 0;
	else if (new_size > CAM_MAX_DEV_OPENINGS)
		new_size = CAM_MAX_DEV_OPENINGS;

	delta = new_size - (ccbq->dev_active + ccbq->dev_openings);
	ccbq->total_openings += delta;
	ccbq->dev_openings += delta;

	new_size = imax(64, 1 << fls(new_size + new_size / 2));
	if (new_size > ccbq->queue.array_size)
		return (camq_resize(&ccbq->queue, new_size));
	else
		return (CAM_REQ_CMP);
}

int
cam_ccbq_init(struct cam_ccbq *ccbq, int openings)
{
	bzero(ccbq, sizeof(*ccbq));
	/*
	 * PBSD: the same clamp as cam_ccbq_resize(), because this is the
	 * same expression. Both callers today pass a SIM's own
	 * max_dev_openings, so this one is not reachable from userland -
	 * but it is exported in cam_queue.h beside the function that is,
	 * and fixing one of a pair is the defect this tree keeps finding.
	 */
	if (openings < 0)
		openings = 0;
	else if (openings > CAM_MAX_DEV_OPENINGS)
		openings = CAM_MAX_DEV_OPENINGS;
	if (camq_init(&ccbq->queue,
	    imax(64, 1 << fls(openings + openings / 2))) != 0)
		return (1);
	ccbq->total_openings = openings;
	ccbq->dev_openings = openings;
	return (0);
}

void
cam_ccbq_fini(struct cam_ccbq *ccbq)
{

	camq_fini(&ccbq->queue);
}

/*
 * Heap routines for manipulating CAM queues.
 */
/*
 * queue_cmp: Given an array of cam_pinfo* elements and indexes i
 * and j, return less than 0, 0, or greater than 0 if i is less than,
 * equal too, or greater than j respectively.
 */
static __inline int
queue_cmp(cam_pinfo **queue_array, int i, int j)
{
	if (queue_array[i]->priority == queue_array[j]->priority)
		return (  queue_array[i]->generation
			- queue_array[j]->generation );
	else
		return (  queue_array[i]->priority
			- queue_array[j]->priority );
}

/*
 * swap: Given an array of cam_pinfo* elements and indexes i and j,
 * exchange elements i and j.
 */
static __inline void
swap(cam_pinfo **queue_array, int i, int j)
{
	cam_pinfo *temp_qentry;

	temp_qentry = queue_array[j];
	queue_array[j] = queue_array[i];
	queue_array[i] = temp_qentry;
	queue_array[j]->index = j;
	queue_array[i]->index = i;
}

/*
 * heap_up:  Given an array of cam_pinfo* elements with the
 * Heap(1, new_index-1) property and a new element in location
 * new_index, output Heap(1, new_index).
 */
static void
heap_up(cam_pinfo **queue_array, int new_index)
{
	int child;
	int parent;

	child = new_index;

	while (child != 1) {
		parent = child >> 1;
		if (queue_cmp(queue_array, parent, child) <= 0)
			break;
		swap(queue_array, parent, child);
		child = parent;
	}
}

/*
 * heap_down:  Given an array of cam_pinfo* elements with the
 * Heap(index + 1, num_entries) property with index containing
 * an unsorted entry, output Heap(index, num_entries).
 */
static void
heap_down(cam_pinfo **queue_array, int index, int num_entries)
{
	int child;
	int parent;

	parent = index;
	child = parent << 1;
	for (; child <= num_entries; child = parent << 1) {
		if (child < num_entries) {
			/* child+1 is the right child of parent */
			if (queue_cmp(queue_array, child + 1, child) < 0)
				child++;
		}
		/* child is now the least child of parent */
		if (queue_cmp(queue_array, parent, child) <= 0)
			break;
		swap(queue_array, child, parent);
		parent = child;
	}
}
