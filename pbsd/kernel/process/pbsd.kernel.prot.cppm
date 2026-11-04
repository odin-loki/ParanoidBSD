export module pbsd.kernel.prot;

import pbsd.core;

/// Freestanding port of `kern_prot.c` visibility / `cr_bsd_visible` policy.
export namespace pbsd::kernel::prot {

struct Ucred {
    unsigned int ruid{0};
    unsigned int rgid{0};
    unsigned int ngroups{0};
    unsigned int groups[16]{};
    unsigned int jail_id{0};
};

/// `kern_prot.c` defaults — PAX_HARDENING uses restrictive see_other_*.
struct BsdVisibilityPolicy {
    bool see_other_uids{false};
    bool see_other_gids{false};
    bool see_jail_proc{false};
    bool has_priv_seeotheruids{false};
    bool has_priv_seeothergids{false};
    bool has_priv_seejailproc{false};
};

[[nodiscard]] inline bool realgroupmember(unsigned gid, const Ucred& subject) noexcept {
    if (gid == subject.rgid) {
        return true;
    }
    for (unsigned i = 0; i < subject.ngroups && i < 16; ++i) {
        if (subject.groups[i] == gid) {
            return true;
        }
    }
    return false;
}

/// `cr_canseeotheruids` — ESRCH → Status::Denied
[[nodiscard]] inline Status can_see_other_uids(const Ucred& viewer, const Ucred& subject,
                                               const BsdVisibilityPolicy& pol) noexcept {
    if (!pol.see_other_uids && viewer.ruid != subject.ruid) {
        if (!pol.has_priv_seeotheruids) {
            return Status::Denied;
        }
    }
    return Status::Ok;
}

/// `cr_canseeothergids`
[[nodiscard]] inline Status can_see_other_gids(const Ucred& viewer, const Ucred& subject,
                                               const BsdVisibilityPolicy& pol) noexcept {
    if (pol.see_other_gids) {
        return Status::Ok;
    }
    if (realgroupmember(viewer.rgid, subject)) {
        return Status::Ok;
    }
    for (unsigned i = 0; i < viewer.ngroups && i < 16; ++i) {
        if (realgroupmember(viewer.groups[i], subject)) {
            return Status::Ok;
        }
    }
    if (pol.has_priv_seeothergids) {
        return Status::Ok;
    }
    return Status::Denied;
}

/// `cr_canseejailproc`
[[nodiscard]] inline Status can_see_jail_proc(const Ucred& viewer, const Ucred& subject,
                                            const BsdVisibilityPolicy& pol) noexcept {
    if (pol.see_jail_proc || viewer.jail_id == subject.jail_id ||
        pol.has_priv_seejailproc) {
        return Status::Ok;
    }
    return Status::Denied;
}

/// `cr_bsd_visible` — aggregate security.bsd.see_* checks.
[[nodiscard]] inline Status bsd_visible(const Ucred& viewer, const Ucred& subject,
                                        const BsdVisibilityPolicy& pol) noexcept {
    if (can_see_other_uids(viewer, subject, pol) != Status::Ok) {
        return Status::Denied;
    }
    if (can_see_other_gids(viewer, subject, pol) != Status::Ok) {
        return Status::Denied;
    }
    if (can_see_jail_proc(viewer, subject, pol) != Status::Ok) {
        return Status::Denied;
    }
    return Status::Ok;
}

/// `p_cansee` — thread/process wrapper over `cr_cansee` BSD leg.
[[nodiscard]] inline Status p_cansee(const Ucred& viewer, const Ucred& target,
                                     const BsdVisibilityPolicy& pol) noexcept {
    if (viewer.jail_id != target.jail_id) {
        return Status::Denied;
    }
    return bsd_visible(viewer, target, pol);
}

/// PAX_HARDENING defaults from `kern_prot.c` (see_other_* = 0).
[[nodiscard]] inline BsdVisibilityPolicy hardened_visibility_policy() noexcept {
    BsdVisibilityPolicy pol{};
    pol.see_other_uids = false;
    pol.see_other_gids = false;
    pol.see_jail_proc  = false;
    return pol;
}

/// Stock FreeBSD defaults (see_other_* = 1).
[[nodiscard]] inline BsdVisibilityPolicy open_visibility_policy() noexcept {
    BsdVisibilityPolicy pol{};
    pol.see_other_uids = true;
    pol.see_other_gids = true;
    pol.see_jail_proc  = true;
    return pol;
}

} // namespace pbsd::kernel::prot
