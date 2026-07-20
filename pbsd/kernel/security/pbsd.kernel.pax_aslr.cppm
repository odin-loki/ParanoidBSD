module;
#include <cstdint>

export module pbsd.kernel.pax_aslr;

import pbsd.core;
import pbsd.kernel.pax_mac;

/// PaX ASLR constants and delta algebra from `hbsd_pax_aslr.c`.
export namespace pbsd::kernel::security::aslr {

inline constexpr unsigned kPageShift = 12u;

/// `PAX_ASLR_DELTA(delta, lsb, len)` — hbsd_pax_aslr.c:60-61
[[nodiscard]] constexpr std::uint64_t aslr_delta(std::uint64_t delta, unsigned lsb,
                                                 unsigned len) noexcept {
    return ((delta & ((1ull << len) - 1ull)) << lsb);
}

// LSB defaults — hbsd_pax_aslr.c
inline constexpr unsigned kDeltaMmapLsb           = kPageShift;
inline constexpr unsigned kDeltaRtldLsb           = kPageShift;
inline constexpr unsigned kDeltaStackLsb          = kPageShift;
inline constexpr unsigned kDeltaThrStackLsb       = kPageShift;
inline constexpr unsigned kDeltaStackWithGapLsb   = 3u;
inline constexpr unsigned kDeltaExecLsb           = kPageShift;
inline constexpr unsigned kDeltaVdsoLsb           = kPageShift;
inline constexpr unsigned kDeltaMap32bitLsb       = kPageShift;

// 64-bit default entropy lengths — hbsd_pax_aslr.c (__LP64__)
inline constexpr unsigned kDeltaMmapDefLen      = 30u;
inline constexpr unsigned kDeltaRtldDefLen      = 30u;
inline constexpr unsigned kDeltaStackDefLen     = 42u;
inline constexpr unsigned kDeltaThrStackDefLen  = 30u;
inline constexpr unsigned kDeltaExecDefLen      = 30u;
inline constexpr unsigned kDeltaVdsoDefLen      = 28u;
inline constexpr unsigned kDeltaMap32bitDefLen  = 18u;

struct AslrDeltas {
    std::uint64_t mmap{};
    std::uint64_t rtld{};
    std::uint64_t stack{};
    std::uint64_t thr_stack{};
    std::uint64_t exec{};
    std::uint64_t vdso{};

    [[nodiscard]] static AslrDeltas from_random(std::uint64_t seed) noexcept {
        AslrDeltas d{};
        d.mmap       = aslr_delta(seed,       kDeltaMmapLsb,     kDeltaMmapDefLen);
        d.rtld       = aslr_delta(seed >> 3,  kDeltaRtldLsb,     kDeltaRtldDefLen);
        d.exec       = aslr_delta(seed >> 7,  kDeltaExecLsb,     kDeltaExecDefLen);
        d.thr_stack  = aslr_delta(seed >> 11, kDeltaThrStackLsb, kDeltaThrStackDefLen);
        d.stack      = aslr_delta(seed >> 13, kDeltaStackWithGapLsb, kDeltaStackDefLen);
        d.vdso       = aslr_delta(seed >> 17, kDeltaVdsoLsb,     kDeltaVdsoDefLen);
        return d;
    }
};

/// `pax_aslr_active` — note flags + default-on policy.
[[nodiscard]] inline bool aslr_active(const PaxProcessFlags& proc) noexcept {
    if (has_note(proc.notes, kPaxNoteAslr)) {
        return true;
    }
    if (has_note(proc.notes, kPaxNoteNoaslr)) {
        return false;
    }
    return true;
}

// vm/mman.h MAP_* bits used by pax_aslr_mmap
inline constexpr int kMapAnon  = 0x1000;
inline constexpr int kMapFixed = 0x0010;

/// `pax_aslr_mmap` — apply mmap delta when hint is zero or mapping is file-backed.
[[nodiscard]] inline std::uint64_t apply_mmap_delta(std::uint64_t addr,
                                                    std::uint64_t orig_addr,
                                                    int mmap_flags,
                                                    std::uint64_t delta_mmap,
                                                    bool active) noexcept {
    if (!active) {
        return addr;
    }
    if ((mmap_flags & kMapFixed) != 0) {
        return addr;
    }
    if (orig_addr == 0 || (mmap_flags & kMapAnon) == 0) {
        return addr + delta_mmap;
    }
    return addr;
}

/// `pax_aslr_stack` — subtract page-aligned stack randomization.
[[nodiscard]] inline std::uint64_t apply_stack_delta(std::uint64_t addr,
                                                     std::uint64_t delta_stack,
                                                     bool active) noexcept {
    if (!active) {
        return addr;
    }
    const std::uint64_t random = delta_stack & (~0ull << kDeltaStackLsb);
    return addr - random;
}

/// `pax_aslr_rtld` — add rtld base delta.
[[nodiscard]] inline std::uint64_t apply_rtld_delta(std::uint64_t addr,
                                                    std::uint64_t delta_rtld,
                                                    bool active) noexcept {
    return active ? (addr + delta_rtld) : addr;
}

} // namespace pbsd::kernel::security::aslr
