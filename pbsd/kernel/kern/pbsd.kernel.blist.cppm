module;
#include <cstdint>

export module pbsd.kernel.blist;

export import pbsd.core;

/// Freestanding port of `sys/blist.h` / `kern/subr_blist.c`.
export namespace pbsd::kernel::blist {

inline constexpr unsigned kMetaSize = 64;

struct Softc {
    unsigned* map{nullptr};
    unsigned  blocks{};
    unsigned  shift{};
};

[[nodiscard]] inline Status init(Softc& b, unsigned blocks) noexcept {
    if (blocks == 0) {
        return Status::Invalid;
    }
    b.blocks = blocks;
    b.shift = 5;
    return Status::Ok;
}

[[nodiscard]] inline unsigned block_index(const Softc& b, unsigned blk) noexcept {
    return blk >> b.shift;
}

} // namespace pbsd::kernel::blist
