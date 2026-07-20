module;
#include <cstdint>

export module pbsd.kernel.kern_jail;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/kern/kern_jail.c — jail id scaffold.
export namespace pbsd::kernel::kern_jail {

struct Jail {
    std::uint32_t jid{};
    bool active{false};
    bool persist{false};
};

[[nodiscard]] inline Status attach(Jail& j, std::uint32_t jid) noexcept {
    if (jid == 0) {
        return Status::Invalid;
    }
    j.jid = jid;
    j.active = true;
    return Status::Ok;
}

[[nodiscard]] inline Status detach(Jail& j) noexcept {
    if (!j.active) {
        return Status::Invalid;
    }
    if (j.persist) {
        j.active = false;
        return Status::Ok;
    }
    j = Jail{};
    return Status::Ok;
}

} // namespace pbsd::kernel::kern_jail
