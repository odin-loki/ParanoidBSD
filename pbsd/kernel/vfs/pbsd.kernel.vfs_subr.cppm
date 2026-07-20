module;

export module pbsd.kernel.vfs_subr;

export import pbsd.core;
import pbsd.kernel.vfs_mount;

/// Freestanding port of `kern/vfs_subr.c` — mount list helpers.
export namespace pbsd::kernel::vfs_subr {

inline constexpr unsigned kRootMount = 0x0001;
inline constexpr unsigned kForce = 0x0002;

[[nodiscard]] inline bool is_root_mount(unsigned flags) noexcept {
    return (flags & kRootMount) != 0;
}

[[nodiscard]] inline Status validate_unmount_flags(unsigned flags) noexcept {
    if (flags & ~(kRootMount | kForce)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::vfs_subr
