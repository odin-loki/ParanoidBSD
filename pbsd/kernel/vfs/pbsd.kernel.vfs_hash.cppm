module;
#include <cstdint>

export module pbsd.kernel.vfs_hash;

export import pbsd.core;
import pbsd.kernel.hash;

/// Freestanding port of `kern/vfs_hash.c` — vnode hash buckets.
export namespace pbsd::kernel::vfs_hash {

inline constexpr unsigned kBucketCount = 256;

[[nodiscard]] inline unsigned bucket_index(std::uint32_t hash) noexcept {
    return hash % kBucketCount;
}

[[nodiscard]] inline std::uint32_t vnode_hash(std::uint64_t ino, unsigned gen) noexcept {
    return pbsd::kernel::hash::step(
        pbsd::kernel::hash::step(pbsd::kernel::hash::kInit,
                                 static_cast<unsigned char>(ino & 0xff)),
        static_cast<unsigned char>(gen & 0xff));
}

} // namespace pbsd::kernel::vfs_hash
