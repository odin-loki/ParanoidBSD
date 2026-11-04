module;

export module pbsd.kernel.security_bsd;

import pbsd.core;
import pbsd.kernel.prot;

/// Freestanding port of `kern_prot.c` security.bsd.* sysctl defaults (PAX_HARDENING).
export namespace pbsd::kernel::security_bsd {

struct SecurityBsdSysctl {
    bool see_other_uids{false};
    bool see_other_gids{false};
    bool see_jail_proc{false};
    bool suser_enabled{true};
    bool unprivileged_chroot{false};
};

[[nodiscard]] inline SecurityBsdSysctl hardened_defaults() noexcept {
    SecurityBsdSysctl s{};
    s.see_other_uids = false;
    s.see_other_gids = false;
    s.see_jail_proc  = false;
    return s;
}

[[nodiscard]] inline SecurityBsdSysctl open_defaults() noexcept {
    SecurityBsdSysctl s{};
    s.see_other_uids = true;
    s.see_other_gids = true;
    s.see_jail_proc  = true;
    return s;
}

[[nodiscard]] inline prot::BsdVisibilityPolicy to_visibility_policy(
    const SecurityBsdSysctl& s,
    bool priv_seeotheruids = false,
    bool priv_seeothergids = false,
    bool priv_seejailproc = false) noexcept {
    prot::BsdVisibilityPolicy pol{};
    pol.see_other_uids = s.see_other_uids;
    pol.see_other_gids = s.see_other_gids;
    pol.see_jail_proc  = s.see_jail_proc;
    pol.has_priv_seeotheruids = priv_seeotheruids;
    pol.has_priv_seeothergids = priv_seeothergids;
    pol.has_priv_seejailproc    = priv_seejailproc;
    return pol;
}

struct SecurityBsdOid {
    const char* name;
    bool SecurityBsdSysctl::* field;
};

inline constexpr SecurityBsdOid kSecurityBsdTable[] = {
    {"see_other_uids", &SecurityBsdSysctl::see_other_uids},
    {"see_other_gids", &SecurityBsdSysctl::see_other_gids},
    {"see_jail_proc",  &SecurityBsdSysctl::see_jail_proc},
    {"suser_enabled",  &SecurityBsdSysctl::suser_enabled},
    {"unprivileged_chroot", &SecurityBsdSysctl::unprivileged_chroot},
};

[[nodiscard]] inline unsigned security_bsd_table_size() noexcept {
    return static_cast<unsigned>(sizeof(kSecurityBsdTable) / sizeof(kSecurityBsdTable[0]));
}

[[nodiscard]] inline Result<bool*> lookup_mutable(SecurityBsdSysctl& s,
                                                  const char* name) noexcept {
    if (name == nullptr) {
        return result_err<bool*>(Status::Invalid);
    }
    for (const auto& e : kSecurityBsdTable) {
        const char* a = name;
        const char* b = e.name;
        while (*a && *b && *a == *b) {
            ++a;
            ++b;
        }
        if (*a == '\0' && *b == '\0') {
            return result_ok(&(s.*(e.field)));
        }
    }
    return result_err<bool*>(Status::NotFound);
}

} // namespace pbsd::kernel::security_bsd
