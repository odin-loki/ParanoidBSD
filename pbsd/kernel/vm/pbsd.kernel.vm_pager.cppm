module;
#include <cstdint>

export module pbsd.kernel.vm_pager;

import pbsd.core;

/// Freestanding port of `vm_pager.h` pager return codes + put flags.
export namespace pbsd::kernel::vm_pager {

inline constexpr int kPagerOk    = 0;
inline constexpr int kPagerBad   = 1;
inline constexpr int kPagerFail  = 2;
inline constexpr int kPagerPend  = 3;
inline constexpr int kPagerError = 4;
inline constexpr int kPagerAgain = 5;

inline constexpr unsigned kPutSync    = 0x0001u;
inline constexpr unsigned kPutInval   = 0x0002u;
inline constexpr unsigned kPutNoreuse = 0x0004u;
inline constexpr unsigned kClusterOk  = 0x0008u;

enum class PagerKind : unsigned char {
    Default = 0,
    Swap    = 1,
    Vnode   = 2,
    Device  = 3,
    Phys    = 4,
    Sg      = 5,
};

struct PagerOpsTable {
    PagerKind kind;
    const char* name;
};

inline constexpr PagerOpsTable kPagerOpsTable[] = {
    {PagerKind::Default, "default"},
    {PagerKind::Swap,    "swap"},
    {PagerKind::Vnode,   "vnode"},
    {PagerKind::Device,  "device"},
    {PagerKind::Phys,    "phys"},
    {PagerKind::Sg,      "sg"},
};

[[nodiscard]] inline unsigned pager_table_size() noexcept {
    return static_cast<unsigned>(sizeof(kPagerOpsTable) / sizeof(kPagerOpsTable[0]));
}

[[nodiscard]] constexpr Status pager_status(int code) noexcept {
    switch (code) {
    case kPagerOk:
        return Status::Ok;
    case kPagerBad:
        return Status::Invalid;
    case kPagerFail:
        return Status::NotFound;
    case kPagerPend:
    case kPagerAgain:
        return Status::Busy;
    case kPagerError:
        return Status::Protocol;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] constexpr bool pager_ok(int code) noexcept {
    return code == kPagerOk;
}

[[nodiscard]] constexpr Status validate_put_flags(unsigned flags) noexcept {
    const unsigned allowed = kPutSync | kPutInval | kPutNoreuse | kClusterOk;
    if ((flags & ~allowed) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::vm_pager
