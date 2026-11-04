/*
 * Batch b0232 oracle: original HardenedBSD C sources concatenated.
 * Functions renamed with ref_ prefix; bodies otherwise unmodified.
 */

#include <stddef.h>
#include <stdlib.h>

#ifndef __weak_reference
#define	__weak_reference(sym, alias)
#endif

/* Harness-visible mock __free (both port and oracle call this). */

static void *mock_last_ptr;
static unsigned mock_call_count;

void
pbsd_b0232_mock_reset(void)
{

	mock_last_ptr = (void *)1;
	mock_call_count = 0;
}

void *
pbsd_b0232_mock_last_ptr(void)
{

	return (mock_last_ptr);
}

unsigned
pbsd_b0232_mock_call_count(void)
{

	return (mock_call_count);
}

void
__free(void *ptr)
{

	mock_last_ptr = ptr;
	mock_call_count++;
	if (ptr != NULL)
		free(ptr);
}

/*
 * Copyright (c) 2026 Faraz Vahedi <kfv@kfv.io>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

void
ref___free_sized(void *ptr, size_t size)
{

	(void)size;
	__free(ptr);
}

__weak_reference(ref___free_sized, free_sized);

/*
 * Copyright (c) 2026 Faraz Vahedi <kfv@kfv.io>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

void
ref___free_aligned_sized(void *ptr, size_t alignment, size_t size)
{

	(void)alignment;
	(void)size;
	__free(ptr);
}

__weak_reference(ref___free_aligned_sized, free_aligned_sized);
