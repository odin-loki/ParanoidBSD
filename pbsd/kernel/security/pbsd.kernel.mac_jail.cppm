module;

export module pbsd.kernel.mac_jail;

import pbsd.core;

/// Freestanding port of `security/mac/mac_jail.c` — MAC jail destroy hook surface.
export namespace pbsd::kernel::mac_jail {

struct PolicyContext {
    bool denied{};
};

struct PrisonStub {
    int  id{};
    bool active{};
    bool mac_notified{};
};

[[nodiscard]] inline Status prison_destroy(PolicyContext& ctx, PrisonStub& pr) noexcept {
    if (!pr.active) {
        return Status::Invalid;
    }
    if (ctx.denied) {
        return Status::Denied;
    }
    pr.mac_notified = true;
    pr.active = false;
    return Status::Ok;
}

} // namespace pbsd::kernel::mac_jail
