module;

export module pbsd.kernel.mac_priv;

import pbsd.core;

/// Freestanding port of `security/mac/mac_priv.c` — MAC privilege checks.
export namespace pbsd::kernel::mac_priv {

enum class Priv : unsigned char {
    Override = 0,
    Audit = 1,
    Admin = 2,
};

[[nodiscard]] inline Status validate_priv(Priv p) noexcept {
    switch (p) {
    case Priv::Override:
    case Priv::Audit:
    case Priv::Admin:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline Status grant(Priv p, bool cred_has_priv) noexcept {
    if (validate_priv(p) != Status::Ok) {
        return Status::Invalid;
    }
    return cred_has_priv ? Status::Ok : Status::Denied;
}

} // namespace pbsd::kernel::mac_priv
