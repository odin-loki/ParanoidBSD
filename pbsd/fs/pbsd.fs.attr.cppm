module;
#include <cstdint>

export module pbsd.fs.attr;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/sys/extattr.h — EXTATTR_NAMESPACE_*.
export namespace pbsd::fs::attr {

enum class Namespace : unsigned int {
    Empty  = 0x00000000,
    User   = 0x00000001,
    System = 0x00000002,
};

inline constexpr unsigned kMaxNameLen = 255;

[[nodiscard]] inline Status validate_namespace(Namespace ns) noexcept {
    switch (ns) {
    case Namespace::Empty:
    case Namespace::User:
    case Namespace::System:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline const char* namespace_string(Namespace ns) noexcept {
    switch (ns) {
    case Namespace::Empty:  return "empty";
    case Namespace::User:   return "user";
    case Namespace::System: return "system";
    default:                return nullptr;
    }
}

} // namespace pbsd::fs::attr
