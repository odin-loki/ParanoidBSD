/*
 * Differential test harness for batch b0194s3
 * (hbsd/src/lib/libc/db/hash/hash.c).
 *
 * Every case is run twice: once through the C++23 port and once through the
 * unmodified C oracle.  Compared per case:
 *
 *   - the returned pointer's null-ness (the returned address itself is a fresh
 *     allocation, so raw addresses are never compared);
 *   - the value written back through p_ptr, expressed as "did *p_ptr become the
 *     returned block, or stay the old block", i.e. an offset-like relation
 *     rather than an absolute address;
 *   - the ENTIRE newsize-byte destination block, byte for byte, between port
 *     and oracle, and additionally against an independent model of the
 *     documented effect (oldsize bytes copied, [oldsize, newsize) zeroed).
 *     The model check is what makes a missing/misplaced memset visible even
 *     when both implementations happen to be handed the same recycled heap
 *     block by the allocator.
 *
 * The source block handed to the function is allocated oldsize + GUARD_TAIL
 * bytes and filled with the guard byte 0x7f before the input is copied in, so
 * a copy that runs past oldsize shows up as 0x7f where the model expects a
 * zero.
 *
 * Before each call the size class the function is about to request is primed
 * with 0xa5-filled, freed blocks.  Without that, an omitted memset could be
 * hidden by fresh zero pages coming from the kernel.
 *
 * newsize < oldsize is never exercised: hash_realloc copies oldsize bytes into
 * a newsize block and then memsets (size_t)(newsize - oldsize) bytes, so such a
 * call is unconditionally out of bounds in the original C too.  Every other
 * shape - including newsize == oldsize and newsize == oldsize + 1 - is
 * exercised on both sides of the boundary.
 */

import pbsd.lib.libc.db.hash.b0194s3;

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <random>

namespace pt = pbsd::lib_libc_db_hash::b0194s3;

extern "C" {
struct oracle_bufhead;
typedef struct oracle_bufhead **ORACLE_SEGMENT;
void *ref_hash_realloc(ORACLE_SEGMENT **p_ptr, int oldsize, int newsize);
}

static const unsigned char GUARD = 0x7f;
static const unsigned char DIRTY = 0xa5;
static const int GUARD_TAIL = 32;

struct fnstat {
	const char *name;
	long cases;
	long failures;
};

static fnstat st_hash_realloc = { "hash_realloc", 0, 0 };

static int reported = 0;

static void
report(const fnstat &st, const char *label, int oldsize, int newsize,
    const char *what)
{
	if (reported >= 20)
		return;
	reported++;
	std::printf("FAIL %s [%s] oldsize=%d newsize=%d: %s\n",
	    st.name, label, oldsize, newsize, what);
}

/*
 * Fill the allocator's free lists for this size class with garbage, so that a
 * block handed back by malloc() is never accidentally zero.
 */
static void
dirty_heap(int size)
{
	void *blk[8];
	std::size_t n = (std::size_t)(size > 0 ? size : 1);
	int i;

	for (i = 0; i < 8; i++) {
		blk[i] = std::malloc(n);
		if (blk[i] == nullptr)
			continue;
		std::memset(blk[i], DIRTY, n);
	}
	for (i = 0; i < 8; i++)
		std::free(blk[i]);
}

/*
 * The documented effect of hash_realloc on its destination block.
 */
static bool
model_ok(const unsigned char *buf, const unsigned char *src, int oldsize,
    int newsize, int *bad)
{
	int copied = oldsize < newsize ? oldsize : newsize;
	int i;

	for (i = 0; i < copied; i++)
		if (buf[i] != src[i]) {
			*bad = i;
			return false;
		}
	for (i = copied; i < newsize; i++)
		if (buf[i] != 0) {
			*bad = i;
			return false;
		}
	return true;
}

static void
case_hash_realloc(const unsigned char *src, int oldsize, int newsize,
    const char *label)
{
	fnstat &st = st_hash_realloc;
	std::size_t alloc = (std::size_t)oldsize + (std::size_t)GUARD_TAIL;
	unsigned char *old_p;
	unsigned char *old_o;
	pt::SEGMENT *ptr_p;
	ORACLE_SEGMENT *ptr_o;
	void *ret_p;
	void *ret_o;
	bool ok = true;
	int bad = -1;

	st.cases++;

	old_p = (unsigned char *)std::malloc(alloc);
	old_o = (unsigned char *)std::malloc(alloc);
	if (old_p == nullptr || old_o == nullptr) {
		st.failures++;
		report(st, label, oldsize, newsize, "test allocation failed");
		std::free(old_p);
		std::free(old_o);
		return;
	}
	std::memset(old_p, GUARD, alloc);
	std::memset(old_o, GUARD, alloc);
	if (oldsize > 0) {
		std::memcpy(old_p, src, (std::size_t)oldsize);
		std::memcpy(old_o, src, (std::size_t)oldsize);
	}
	if (std::memcmp(old_p, old_o, alloc) != 0) {
		st.failures++;
		report(st, label, oldsize, newsize, "inputs differ");
		std::free(old_p);
		std::free(old_o);
		return;
	}

	ptr_p = (pt::SEGMENT *)old_p;
	ptr_o = (ORACLE_SEGMENT *)old_o;

	dirty_heap(newsize);
	ret_p = pt::hash_realloc(&ptr_p, oldsize, newsize);
	dirty_heap(newsize);
	ret_o = ref_hash_realloc(&ptr_o, oldsize, newsize);

	if ((ret_p == nullptr) != (ret_o == nullptr)) {
		ok = false;
		report(st, label, oldsize, newsize, "return null-ness differs");
	}

	/* *p_ptr must have been retargeted at the new block, or left alone. */
	if (ret_p != nullptr) {
		if ((void *)ptr_p != ret_p) {
			ok = false;
			report(st, label, oldsize, newsize,
			    "port did not store the new block in *p_ptr");
		}
	} else if ((void *)ptr_p != (void *)old_p) {
		ok = false;
		report(st, label, oldsize, newsize,
		    "port clobbered *p_ptr on allocation failure");
	}
	if (ret_o != nullptr) {
		if ((void *)ptr_o != ret_o) {
			ok = false;
			report(st, label, oldsize, newsize,
			    "oracle did not store the new block in *p_ptr");
		}
	} else if ((void *)ptr_o != (void *)old_o) {
		ok = false;
		report(st, label, oldsize, newsize,
		    "oracle clobbered *p_ptr on allocation failure");
	}

	if (ret_p != nullptr && ret_o != nullptr && newsize > 0) {
		if (std::memcmp(ret_p, ret_o, (std::size_t)newsize) != 0) {
			ok = false;
			report(st, label, oldsize, newsize,
			    "destination blocks differ");
		}
		if (!model_ok((const unsigned char *)ret_p, src, oldsize,
		    newsize, &bad)) {
			ok = false;
			report(st, label, oldsize, newsize,
			    "port destination does not match model");
		}
		if (!model_ok((const unsigned char *)ret_o, src, oldsize,
		    newsize, &bad)) {
			ok = false;
			report(st, label, oldsize, newsize,
			    "oracle destination does not match model");
		}
	}

	if (ret_p != nullptr)
		std::free(ret_p);
	else
		std::free(old_p);
	if (ret_o != nullptr)
		std::free(ret_o);
	else
		std::free(old_o);

	if (!ok)
		st.failures++;
}

static void
fill_pattern(unsigned char *buf, int len, int pattern)
{
	int i;

	for (i = 0; i < len; i++) {
		switch (pattern) {
		case 0: buf[i] = 0x00; break;
		case 1: buf[i] = 0xff; break;
		case 2: buf[i] = 0x80; break;
		case 3: buf[i] = 0x7f; break;
		case 4: buf[i] = (unsigned char)(i & 0xff); break;
		case 5: buf[i] = (unsigned char)(0x80 + (i & 0x7f)); break;
		case 6: buf[i] = (unsigned char)((i & 1) ? 0x00 : 0xff); break;
		case 7: buf[i] = (unsigned char)((i % 3) == 0 ? 0x00 : 0xa5);
			break;
		default: buf[i] = (unsigned char)(0xff - (i & 0xff)); break;
		}
	}
}

#define MAXEDGE 4097
#define NPATTERN 9

static void
edge_cases(void)
{
	static const int sizes[] = {
		0, 1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65,
		127, 128, 129, 255, 256, 257, 511, 512, 1024, 2047, 2048, 4095,
		4096, 4097
	};
	unsigned char *src = (unsigned char *)std::malloc(MAXEDGE);
	std::size_t i;
	int p;

	if (src == nullptr) {
		std::printf("edge_cases: out of memory\n");
		std::exit(1);
	}
	for (i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
		int oldsize = sizes[i];

		for (p = 0; p < NPATTERN; p++) {
			fill_pattern(src, oldsize, p);
			/* newsize == oldsize: zero-length memset boundary */
			case_hash_realloc(src, oldsize, oldsize, "equal");
			/* one byte of tail: smallest non-empty memset */
			case_hash_realloc(src, oldsize, oldsize + 1, "grow1");
			/* two bytes of tail */
			case_hash_realloc(src, oldsize, oldsize + 2, "grow2");
			/* the doubling the real callers use */
			case_hash_realloc(src, oldsize, oldsize * 2, "double");
			/* tail much larger than the copied prefix */
			case_hash_realloc(src, oldsize, oldsize + 4096,
			    "growbig");
		}
	}
	/* oldsize == 0: nothing copied, everything zeroed */
	fill_pattern(src, 1, 1);
	case_hash_realloc(src, 0, 0, "zero/zero");
	case_hash_realloc(src, 0, 1, "zero/one");
	case_hash_realloc(src, 0, 4096, "zero/page");
	std::free(src);
}

#define MAXRAND 512

static void
random_cases(void)
{
	std::mt19937 rng(0xb0194533u);
	unsigned char src[MAXRAND];
	int it;

	for (it = 0; it < 30000; it++) {
		int oldsize = (int)(rng() % (MAXRAND + 1));
		int shape = (int)(rng() % 5);
		int newsize;
		int i;
		int p = (int)(rng() % (NPATTERN + 1));

		switch (shape) {
		case 0: newsize = oldsize; break;
		case 1: newsize = oldsize + 1; break;
		case 2: newsize = oldsize + 2; break;
		case 3: newsize = oldsize * 2; break;
		default: newsize = oldsize + (int)(rng() % 1024); break;
		}

		if (p < NPATTERN)
			fill_pattern(src, oldsize, p);
		else
			for (i = 0; i < oldsize; i++)
				src[i] = (unsigned char)(rng() & 0xff);

		case_hash_realloc(src, oldsize, newsize, "random");
	}
}

int
main(void)
{
	long total_failures;

	edge_cases();
	random_cases();

	total_failures = st_hash_realloc.failures;

	std::printf("\n%-24s %10s %10s\n", "function", "cases", "failures");
	std::printf("%-24s %10s %10s\n", "------------------------",
	    "----------", "----------");
	std::printf("%-24s %10ld %10ld\n", st_hash_realloc.name,
	    st_hash_realloc.cases, st_hash_realloc.failures);
	std::printf("\n%s\n", total_failures == 0 ? "ALL CASES MATCHED" :
	    "DIVERGENCE DETECTED");

	return total_failures == 0 ? 0 : 1;
}
