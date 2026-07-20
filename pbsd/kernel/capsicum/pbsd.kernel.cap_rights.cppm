module;
#include <cstdint>

export module pbsd.kernel.cap_rights;

export import pbsd.rights;
import pbsd.core;
import pbsd.kernel.capsicum;

/// Capability-rights glue between pbsd.rights and hbsd caprights (sys/capability.c).
export namespace pbsd::kernel::cap_rights {

using capsicum::CapRights;
using capsicum::cap_check;
using capsicum::cap_none;
using capsicum::cap_rights_contains;
using capsicum::cap_rights_is_set;
using capsicum::kCapIoctl;
using capsicum::kCapRead;
using capsicum::kCapWrite;

[[nodiscard]] inline CapRights from_pbsd_rights(CapabilityRights r) noexcept {
    CapRights out = cap_none();
    if (has_right(r, CapabilityRights::Read)) {
        out.cr_rights[capsicum::right_to_index(kCapRead)] |= kCapRead;
    }
    if (has_right(r, CapabilityRights::Write)) {
        out.cr_rights[capsicum::right_to_index(kCapWrite)] |= kCapWrite;
    }
    if (has_right(r, CapabilityRights::Execute)) {
        out.cr_rights[capsicum::right_to_index(capsicum::kCapMmapX)] |= capsicum::kCapMmapX;
    }
    if (has_right(r, CapabilityRights::Map)) {
        out.cr_rights[capsicum::right_to_index(capsicum::kCapMmap)] |= capsicum::kCapMmap;
    }
    if (has_right(r, CapabilityRights::Grant)) {
        out.cr_rights[capsicum::right_to_index(capsicum::kCapCreate)] |= capsicum::kCapCreate;
    }
    if (has_right(r, CapabilityRights::Destroy)) {
        out.cr_rights[capsicum::right_to_index(capsicum::kCapFtruncate)] |= capsicum::kCapFtruncate;
    }
    if (has_right(r, CapabilityRights::Irq)) {
        out.cr_rights[capsicum::right_to_index(kCapIoctl)] |= kCapIoctl;
    }
    return out;
}

[[nodiscard]] inline CapabilityRights to_pbsd_rights(const CapRights& cr) noexcept {
    CapabilityRights r = CapabilityRights::None;
    if (cap_rights_is_set(cr, kCapRead)) {
        r = r | CapabilityRights::Read;
    }
    if (cap_rights_is_set(cr, kCapWrite)) {
        r = r | CapabilityRights::Write;
    }
    if (cap_rights_is_set(cr, capsicum::kCapMmapX)) {
        r = r | CapabilityRights::Execute;
    }
    if (cap_rights_is_set(cr, capsicum::kCapMmap)) {
        r = r | CapabilityRights::Map;
    }
    if (cap_rights_is_set(cr, capsicum::kCapCreate)) {
        r = r | CapabilityRights::Grant;
    }
    if (cap_rights_is_set(cr, kCapIoctl)) {
        r = r | CapabilityRights::Irq;
    }
    return r;
}

/// `kern_cap_rights_limit` — typed rights must narrow (SI-1).
[[nodiscard]] inline Status limit_rights(CapabilityRights current,
                                         CapabilityRights limit,
                                         CapRights& cap_out) noexcept {
    if (!rights_subset(limit, current)) {
        return Status::Denied;
    }
    cap_out = from_pbsd_rights(limit);
    if (!capsicum::cap_rights_is_valid(cap_out)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

/// `_cap_check` wrapper for syscall dispatch tables.
[[nodiscard]] inline Status check_cap_rights(const CapRights& held,
                                             const CapRights& need) noexcept {
    return cap_check(held, need);
}

[[nodiscard]] inline Status check_pbsd_rights(CapabilityRights held,
                                              CapabilityRights need) noexcept {
    if (!rights_subset(need, held)) {
        return Status::Denied;
    }
    return check_cap_rights(from_pbsd_rights(held), from_pbsd_rights(need));
}

[[nodiscard]] inline bool cap_contains_pbsd(const CapRights& held,
                                          CapabilityRights want) noexcept {
    return cap_rights_contains(held, from_pbsd_rights(want));
}

} // namespace pbsd::kernel::cap_rights
