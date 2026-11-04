module;

export module pbsd.kernel.disk;

export import pbsd.core;

/// Wave 4/5 — disk ioctls from sys/disk.h.
export namespace pbsd::kernel::disk {

inline constexpr unsigned kIdentSize = 256;

enum class Ioctl : unsigned {
    SectorSize   = 128,
    MediaSize    = 129,
    FwSectors    = 130,
    FwHeads      = 131,
    Flush        = 135,
    Delete       = 136,
    Ident        = 137,
    ProviderName = 138,
    StripeSize   = 139,
    StripeOffset = 140,
    PhyPath      = 141,
    Attr         = 142,
    KernelDump   = 146,
};

[[nodiscard]] constexpr Status validate_ident_len(unsigned len) noexcept {
    if (len == 0 || len > kIdentSize) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::disk
