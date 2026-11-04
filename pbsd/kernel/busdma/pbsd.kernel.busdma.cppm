module;
#include <cstdint>

export module pbsd.kernel.busdma;

export import pbsd.core;

/// Wave 4/5 — bus_dma tags from sys/bus_dma.h, kern/subr_busdma.c.
export namespace pbsd::kernel::busdma {

inline constexpr unsigned kBusDmaMaxSegs = 128;
inline constexpr unsigned kBusDmaAlignment = 4096;
inline constexpr unsigned kBusDmaBoundary = 0;

inline constexpr unsigned kBusDmaRead  = 0x01;
inline constexpr unsigned kBusDmaWrite = 0x02;
inline constexpr unsigned kBusDmaZero  = 0x04;
inline constexpr unsigned kBusDmaCoherent = 0x08;

enum class BusDmaDirection : unsigned char {
    ToDevice = 0,
    FromDevice = 1,
    Bidirectional = 2,
};

struct BusDmaTag {
    unsigned max_size{};
    unsigned alignment{kBusDmaAlignment};
    unsigned boundary{kBusDmaBoundary};
    unsigned max_segs{kBusDmaMaxSegs};
    unsigned flags{};
};

struct BusDmaSegment {
    std::uint64_t ds_addr{};
    std::uint64_t ds_len{};
};

[[nodiscard]] constexpr Status validate_tag(const BusDmaTag& tag) noexcept {
    if (tag.max_size == 0 || tag.max_segs == 0 || tag.max_segs > kBusDmaMaxSegs) {
        return Status::Invalid;
    }
    if (tag.alignment == 0 || (tag.alignment & (tag.alignment - 1)) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status validate_segment(const BusDmaSegment& seg) noexcept {
    if (seg.ds_len == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr bool flag_coherent(unsigned flags) noexcept {
    return (flags & kBusDmaCoherent) != 0;
}

} // namespace pbsd::kernel::busdma
