module;
#include <cstdint>

export module pbsd.fs.namei;

import pbsd.core;
import pbsd.rights;

using pbsd::CapabilityRights;
using pbsd::has_right;

/// PROVENANCE: hbsd/src/sys/sys/namei.h — namei lookup operation flags.
export namespace pbsd::fs::namei {

enum class OpFlag : unsigned int {
    LockLeaf    = 0x0004,
    LockParent  = 0x0008,
    WantParent  = 0x0010,
    NoFollow    = 0x0000,
    Follow      = 0x0040,
    Create      = 0x0200,
    Delete      = 0x0400,
    Lookup      = 0x0800,
    Rename      = 0x1000,
    Open        = 0x2000,
    Readdir     = 0x4000,
    Directory   = 0x8000,
};

struct OpEntry {
    OpFlag          op{};
    const char*     label{};
    CapabilityRights need{};
};

inline constexpr OpEntry kOpTable[] = {
    {OpFlag::Lookup, "lookup", CapabilityRights::Read | CapabilityRights::Execute},
    {OpFlag::Create, "create", CapabilityRights::Write},
    {OpFlag::Delete, "delete", CapabilityRights::Write | CapabilityRights::Destroy},
    {OpFlag::Rename, "rename", CapabilityRights::Write},
    {OpFlag::Open, "open", CapabilityRights::Read | CapabilityRights::Write},
    {OpFlag::Readdir, "readdir", CapabilityRights::Read},
    {OpFlag::Directory, "directory", CapabilityRights::Read | CapabilityRights::Execute},
};

[[nodiscard]] inline constexpr std::size_t op_table_size() noexcept {
    return sizeof(kOpTable) / sizeof(kOpTable[0]);
}

[[nodiscard]] inline Status check_op(CapabilityRights held, OpFlag op) noexcept {
    for (auto const& e : kOpTable) {
        if (e.op == op) {
            return has_right(held, e.need) ? Status::Ok : Status::Denied;
        }
    }
    return Status::Invalid;
}

[[nodiscard]] inline bool wants_parent(unsigned flags) noexcept {
    return (flags & (static_cast<unsigned>(OpFlag::LockParent)
                     | static_cast<unsigned>(OpFlag::WantParent))) != 0;
}

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    if ((flags & static_cast<unsigned>(OpFlag::LockParent))
        && (flags & static_cast<unsigned>(OpFlag::WantParent))) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::fs::namei
