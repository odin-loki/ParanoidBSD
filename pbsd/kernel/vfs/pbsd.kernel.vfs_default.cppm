module;

export module pbsd.kernel.vfs_default;

export import pbsd.core;

/// Freestanding port of `kern/vfs_default.c` — default vnode ops stubs.
export namespace pbsd::kernel::vfs_default {

enum class Op : unsigned char {
    Open, Close, Read, Write, Ioctl, Poll, Kqfilter, Revoke, Mmap, Fsync,
    Seek, Remove, Link, Rename, Mkdir, Rmdir, Readdir, Symlink, Readlink,
    Abortop, Access, Getattr, Setattr, Pathconf, Advlock, Reallocblks,
    Strategy, Getwritemount, Suspended, Reclaim, Inactive, Needinval,
    Advlockasync, Vptofh, Vptocnp, Truncate, Copy_file_range, Offload,
    Aio_read, Aio_write, Aio_fsync, Aio_cancel, Aio_error, Aio_return,
    Unknown,
};

[[nodiscard]] inline Status default_not_supported(Op op) noexcept {
    (void)op;
    return Status::Denied;
}

} // namespace pbsd::kernel::vfs_default
