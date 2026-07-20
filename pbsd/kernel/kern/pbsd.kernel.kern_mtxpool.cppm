module;

export module pbsd.kernel.kern_mtxpool;

export import pbsd.core;

/// Freestanding port of `kern/kern_mtxpool.c` — mtxpool helpers.
export namespace pbsd::kernel::kern_mtxpool {

inline constexpr unsigned kMpBuckets = 128;

[[nodiscard]] inline unsigned bucket(unsigned hash) noexcept {
    return hash % kMpBuckets;
}

} // namespace pbsd::kernel::kern_mtxpool
