module;

#include <cstddef>
#include <cstdlib>

export module pbsd.lib.libc.stdlib.malloc.b0232;

export namespace pbsd::lib_libc_stdlib_malloc::b0232 {

extern "C" void __free(void *);

/*
 * Copyright (c) 2026 Faraz Vahedi <kfv@kfv.io>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

void
__free_sized(void *ptr, size_t size)
{

	(void)size;
	__free(ptr);
}

/*
 * Copyright (c) 2026 Faraz Vahedi <kfv@kfv.io>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

void
__free_aligned_sized(void *ptr, size_t alignment, size_t size)
{

	(void)alignment;
	(void)size;
	__free(ptr);
}

} /* namespace pbsd::lib_libc_stdlib_malloc::b0232 */
