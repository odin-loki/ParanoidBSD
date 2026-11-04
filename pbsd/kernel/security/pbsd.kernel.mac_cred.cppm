module;

export module pbsd.kernel.mac_cred;

import pbsd.core;
import pbsd.kernel.mac_label;

/// Freestanding port of `security/mac/mac_cred.c` — credential MAC label helpers.
export namespace pbsd::kernel::mac_cred {

[[nodiscard]] inline Status label_alloc(bool allocated) noexcept {
    return allocated ? Status::Ok : Status::NoMemory;
}

[[nodiscard]] inline Status label_copy(bool src_valid, bool dst_valid) noexcept {
    if (!src_valid) {
        return Status::Invalid;
    }
    if (dst_valid) {
        return Status::Busy;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status label_destroy(bool allocated) noexcept {
    return allocated ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::kernel::mac_cred
