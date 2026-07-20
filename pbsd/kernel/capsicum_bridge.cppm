export module pbsd.kernel.capsicum_bridge;

export import pbsd.handles;
export import pbsd.kernel.capsicum;
import pbsd.core;

/// Bridge between PBSD typed handles and hbsd Capsicum KPI (`sys_capability.c`).
export namespace pbsd::kernel {

using capsicum::CapRights;
using capsicum::cap_check;
using capsicum::cap_none;
using capsicum::cap_rights_contains;
using capsicum::cap_rights_is_set;
using capsicum::cap_rights_to_vmprot;
using capsicum::kCapMmapR;
using capsicum::kCapMmapW;
using capsicum::kCapMmapX;
using capsicum::kCapRead;
using capsicum::kCapWrite;

/// Legacy 6-bit bridge surface (Wave 4).
struct CapRightsBits {
    unsigned long long bits{};
};

[[nodiscard]] inline CapRights encode(CapabilityRights r) noexcept {
    CapRights out = cap_none();
    if (has_right(r, CapabilityRights::Read)) {
        out.cr_rights[capsicum::right_to_index(kCapRead)] |= kCapRead;
    }
    if (has_right(r, CapabilityRights::Write)) {
        out.cr_rights[capsicum::right_to_index(kCapWrite)] |= kCapWrite;
    }
    if (has_right(r, CapabilityRights::Execute)) {
        out.cr_rights[capsicum::right_to_index(kCapMmapX)] |= kCapMmapX;
    }
    if (has_right(r, CapabilityRights::Map)) {
        out.cr_rights[capsicum::right_to_index(capsicum::kCapMmap)] |= capsicum::kCapMmap;
    }
    return out;
}

[[nodiscard]] inline CapRightsBits encode_bits(CapabilityRights r) noexcept {
    return CapRightsBits{encode(r).cr_rights[0]};
}

[[nodiscard]] inline CapabilityRights decode(const CapRights& rights) noexcept {
    CapabilityRights r = CapabilityRights::None;
    if (cap_rights_is_set(rights, kCapRead)) {
        r = r | CapabilityRights::Read;
    }
    if (cap_rights_is_set(rights, kCapWrite)) {
        r = r | CapabilityRights::Write;
    }
    if (cap_rights_is_set(rights, kCapMmapX)) {
        r = r | CapabilityRights::Execute;
    }
    if (cap_rights_is_set(rights, capsicum::kCapMmap)) {
        r = r | CapabilityRights::Map;
    }
    if (cap_rights_is_set(rights, capsicum::kCapMmapR)) {
        r = r | CapabilityRights::Read;
    }
    if (cap_rights_is_set(rights, capsicum::kCapMmapW)) {
        r = r | CapabilityRights::Write;
    }
    return r;
}

[[nodiscard]] inline CapabilityRights from_cap_rights(const CapRights& rights) noexcept {
    return decode(rights);
}

[[nodiscard]] inline CapRights from_cap_rights_words(unsigned long long w0,
                                                     unsigned long long w1) noexcept {
    CapRights r{};
    r.cr_rights[0] = w0;
    r.cr_rights[1] = w1;
    return r;
}

/// `sys_capability.c:_cap_check` hook target for lineage revocation glue.
[[nodiscard]] inline Status revoke_lineage_hook(LineageTree& tree, LineageId id) noexcept {
    tree.revoke(id);
    return Status::Ok;
}

/// `kern_cap_rights_limit` / `_cap_check` — rights must narrow (SI-1).
[[nodiscard]] inline Status limit_fd_rights(const CapRights& current,
                                            const CapRights& limit) noexcept {
    if (!capsicum::cap_rights_is_valid(limit)) {
        return Status::Invalid;
    }
    return cap_check(current, limit);
}

/// Process capability namespace (`struct proc` glue target).
struct ProcCapNamespace {
    LineageId root_lineage{kInvalidLineage};
    bool      in_capability_mode{false};
    CapRights root_rights{cap_none()};
};

/// `sys_cap_enter` policy surface — enter capmode with validated root rights.
[[nodiscard]] inline Status enter_capsicum(ProcCapNamespace& ns, LineageId root,
                                           const CapRights& rights) noexcept {
    if (root == kInvalidLineage || !capsicum::cap_rights_is_valid(rights)) {
        return Status::Invalid;
    }
    ns.root_lineage = root;
    ns.root_rights  = rights;
    ns.in_capability_mode = true;
    return Status::Ok;
}

/// `cap_rights_limit` for PBSD handles — narrow typed rights + cap words.
[[nodiscard]] inline Status limit_handle_rights(CapabilityRights current,
                                                CapabilityRights limit,
                                                CapRights& cap_out) noexcept {
    if (check_grant(current, limit) != Status::Ok) {
        return Status::Denied;
    }
    cap_out = encode(limit);
    return Status::Ok;
}

} // namespace pbsd::kernel
