module;

export module pbsd.kernel.vfs_vnops;

export import pbsd.core;
import pbsd.kernel.vnode;

/// Freestanding port of `kern/vfs_vnops.c` — vn_open/vn_close helpers.
export namespace pbsd::kernel::vfs_vnops {

inline constexpr unsigned kVnShared = 0x0001;
inline constexpr unsigned kVnNoFail = 0x0002;

[[nodiscard]] inline Status validate_vn_flags(unsigned flags) noexcept {
    if (flags & ~(kVnShared | kVnNoFail)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::vfs_vnops
