module;
#include <cstdint>

export module pbsd.arch.amd64.tss;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/include/tss.h — 64-bit TSS fields used by kernel.
export namespace pbsd::arch::amd64::tss {

inline constexpr unsigned kIstSlots = 7;
inline constexpr unsigned kIoMapOffsetUnused = 0xffff;

struct Softc {
    std::uint32_t reserved0{0};
    std::uint64_t rsp0{0};
    std::uint64_t rsp1{0};
    std::uint64_t rsp2{0};
    std::uint64_t ist[kIstSlots]{};
    std::uint16_t iomap_base{kIoMapOffsetUnused};
};

[[nodiscard]] inline Status set_rsp0(Softc& t, std::uint64_t rsp) noexcept {
    if (rsp == 0) {
        return Status::Invalid;
    }
    t.rsp0 = rsp;
    return Status::Ok;
}

[[nodiscard]] inline Status set_ist(Softc& t, unsigned slot, std::uint64_t rsp) noexcept {
    if (slot == 0 || slot > kIstSlots) {
        return Status::Invalid;
    }
    t.ist[slot - 1] = rsp;
    return Status::Ok;
}

[[nodiscard]] inline bool iomap_disabled(const Softc& t) noexcept {
    return t.iomap_base == kIoMapOffsetUnused;
}

} // namespace pbsd::arch::amd64::tss
