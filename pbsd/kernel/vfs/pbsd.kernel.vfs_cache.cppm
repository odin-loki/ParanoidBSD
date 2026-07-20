module;

export module pbsd.kernel.vfs_cache;

export import pbsd.core;

/// Freestanding port of `kern/vfs_cache.c` — name cache flags.
export namespace pbsd::kernel::vfs_cache {

inline constexpr unsigned kCacheFree = 0x0001;
inline constexpr unsigned kCacheDead = 0x0002;
inline constexpr unsigned kCacheNegative = 0x0004;

[[nodiscard]] inline bool is_negative(unsigned flags) noexcept {
    return (flags & kCacheNegative) != 0;
}

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    if (flags & ~(kCacheFree | kCacheDead | kCacheNegative)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::vfs_cache
