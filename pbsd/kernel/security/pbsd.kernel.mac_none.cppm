module;

export module pbsd.kernel.mac_none;

import pbsd.core;

/// Freestanding port of `security/mac_none/mac_none.c` — no-op MAC policy stub.
export namespace pbsd::kernel::mac_none {

inline constexpr char const* kPolicyName = "TrustedBSD MAC/None";
inline constexpr unsigned kUnloadOk    = 0x01;

struct PolicyStub {
    char const* name{kPolicyName};
    unsigned    flags{kUnloadOk};
    bool        loaded{};
};

[[nodiscard]] inline Status load(PolicyStub& pol) noexcept {
    if (pol.loaded) {
        return Status::Busy;
    }
    pol.loaded = true;
    return Status::Ok;
}

[[nodiscard]] inline Status unload(PolicyStub& pol) noexcept {
    if (!pol.loaded) {
        return Status::Invalid;
    }
    if ((pol.flags & kUnloadOk) == 0) {
        return Status::Denied;
    }
    pol.loaded = false;
    return Status::Ok;
}

} // namespace pbsd::kernel::mac_none
