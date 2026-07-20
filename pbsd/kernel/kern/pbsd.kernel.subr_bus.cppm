module;
#include <cstdint>

export module pbsd.kernel.subr_bus;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/kern/subr_bus.c — newbus attach/probe helpers (UDA dual-world).
export namespace pbsd::kernel::subr_bus {

inline constexpr unsigned kMaxDevName = 32;

struct DevName {
    char data[kMaxDevName]{};
};

[[nodiscard]] inline Status validate_name(const char* name) noexcept {
    if (name == nullptr || name[0] == '\0') {
        return Status::Invalid;
    }
    for (unsigned i = 0; i < kMaxDevName && name[i] != '\0'; ++i) {
        const char c = name[i];
        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_') {
            continue;
        }
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status copy_name(DevName& out, const char* name) noexcept {
    if (validate_name(name) != Status::Ok) {
        return Status::Invalid;
    }
    unsigned i = 0;
    for (; i + 1 < kMaxDevName && name[i] != '\0'; ++i) {
        out.data[i] = name[i];
    }
    out.data[i] = '\0';
    return Status::Ok;
}

} // namespace pbsd::kernel::subr_bus
