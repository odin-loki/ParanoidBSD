/*
 * PBSD batch b0008 -- C++23 port of the HardenedBSD libc csu aarch64
 * reloc.c relocation helpers.
 *
 * Original copyright headers are retained above each ported function.
 * Behaviour, signedness and evaluation order are otherwise unchanged.
 *
 * The Elf/ifunc typedefs and constants from <machine/ifunc.h> and the
 * ELF headers are not part of this batch; they are reproduced here as
 * scaffolding so the ported functions compile and behave identically.
 */

module;

#include <cstdint>

export module pbsd.lib.libc.csu.aarch64.b0008;

export namespace pbsd::lib_libc_csu_aarch64::b0008 {

/* ------------------------------------------------------------------ */
/* machine/ifunc.h and ELF scaffolding                                */
/* ------------------------------------------------------------------ */

using Elf_Addr = std::uint64_t;

struct __ifunc_arg_t {
	unsigned long _size;
	unsigned long _hwcap;
	unsigned long _hwcap2;
	unsigned long _hwcap3;
	unsigned long _hwcap4;
};

struct Elf_Auxinfo {
	Elf_Addr a_type;
	union {
		Elf_Addr a_val;
	} a_un;
};

struct Elf_Rela {
	Elf_Addr r_offset;
	Elf_Addr r_info;
	long r_addend;
};

inline constexpr Elf_Addr AT_NULL = 0;
inline constexpr Elf_Addr AT_HWCAP = 16;
inline constexpr Elf_Addr AT_HWCAP2 = 26;
inline constexpr Elf_Addr AT_HWCAP3 = 35;
inline constexpr Elf_Addr AT_HWCAP4 = 38;

inline constexpr unsigned long _IFUNC_ARG_HWCAP = (1ULL << 62);

inline constexpr Elf_Addr R_AARCH64_IRELATIVE = 1032;

inline constexpr Elf_Addr ELF_R_TYPE(Elf_Addr info) noexcept
{
	return info & 0xffffffffUL;
}

__ifunc_arg_t ifunc_arg;

/* ------------------------------------------------------------------ */
/* reloc.c                                                            */
/* ------------------------------------------------------------------ */

/*-
 * Copyright (c) 2019 Leandro Lupori
 * Copyright (c) 2021 The FreeBSD Foundation
 *
 * Portions of this software were developed by Andrew Turner
 * under sponsorship from the FreeBSD Foundation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
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
 */

void ifunc_init(const Elf_Auxinfo *aux) noexcept
{
	ifunc_arg._size = sizeof(ifunc_arg);
	ifunc_arg._hwcap = 0;
	ifunc_arg._hwcap2 = 0;
	ifunc_arg._hwcap3 = 0;
	ifunc_arg._hwcap4 = 0;

	for (;  aux->a_type != AT_NULL; aux++) {
		switch (aux->a_type) {
		case AT_HWCAP:
			ifunc_arg._hwcap = aux->a_un.a_val | _IFUNC_ARG_HWCAP;
			break;
		case AT_HWCAP2:
			ifunc_arg._hwcap2 = aux->a_un.a_val;
			break;
		case AT_HWCAP3:
			ifunc_arg._hwcap3 = aux->a_un.a_val;
			break;
		case AT_HWCAP4:
			ifunc_arg._hwcap4 = aux->a_un.a_val;
			break;
		}
	}
}

void crt1_handle_rela(const Elf_Rela *r) noexcept
{
	typedef Elf_Addr (*ifunc_resolver_t)(
	    std::uint64_t, const __ifunc_arg_t *, std::uint64_t, std::uint64_t,
	    std::uint64_t, std::uint64_t, std::uint64_t, std::uint64_t);
	Elf_Addr *ptr, *where, target;

	switch (ELF_R_TYPE(r->r_info)) {
	case R_AARCH64_IRELATIVE:
		ptr = (Elf_Addr *)r->r_addend;
		where = (Elf_Addr *)r->r_offset;
		target = ((ifunc_resolver_t)ptr)(ifunc_arg._hwcap, &ifunc_arg, 0, 0, 0, 0, 0, 0);
		*where = target;
		break;
	}
}

} /* namespace pbsd::lib_libc_csu_aarch64::b0008 */
