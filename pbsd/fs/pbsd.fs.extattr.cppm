module;
#include <cstdint>

export module pbsd.fs.extattr;

import pbsd.core;
import pbsd.rights;

using pbsd::CapabilityRights;
using pbsd::has_right;

/// PROVENANCE: hbsd/src/sys/sys/extattr.h — extended attribute operations.
export namespace pbsd::fs::extattr {

enum class Op : unsigned char {
    Get    = 0,
    Set    = 1,
    Remove = 2,
    List   = 3,
};

struct Entry {
    int namespace_id{};
    unsigned name_len{};
    unsigned value_len{};
};

[[nodiscard]] inline Status validate_entry(Entry const& e) noexcept {
    if (e.namespace_id <= 0) {
        return Status::Invalid;
    }
    if (e.name_len == 0 || e.name_len > 255) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status check_op(CapabilityRights rights, Op op) noexcept {
    switch (op) {
    case Op::Get:
    case Op::List:
        return has_right(rights, CapabilityRights::Read) ? Status::Ok : Status::Denied;
    case Op::Set:
    case Op::Remove:
        return has_right(rights, CapabilityRights::Write) ? Status::Ok : Status::Denied;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::fs::extattr
