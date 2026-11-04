module;
extern "C" {
typedef unsigned long long pbsd_cap_rights_t;
typedef unsigned long long pbsd_lineage_id_t;
}

export module pbsd.kernel.shim;

import pbsd.core;
import pbsd.kernel.capsicum;
import pbsd.kernel.capsicum_bridge;
import pbsd.kernel.vm;
import pbsd.kernel.pax_mac;
import pbsd.kernel.pax_aslr;
import pbsd.kernel.prot;
import pbsd.kernel.resource;
import pbsd.kernel.sysctl;
import pbsd.kernel.signal;
import pbsd.kernel.errno;
import pbsd.kernel.vm_pager;

/// Dual-link shim module — extern "C" entry points for hbsd KPI.
export extern "C" int pbsd_kernel_shim_init(void) {
    return 0;
}

export extern "C" void pbsd_kernel_shim_fini(void) {}

export extern "C" pbsd_cap_rights_t pbsd_kernel_encode_rights(unsigned read, unsigned write,
                                                              unsigned grant, unsigned exec) {
    using namespace pbsd;
    CapabilityRights r = CapabilityRights::None;
    if (read) {
        r = r | CapabilityRights::Read;
    }
    if (write) {
        r = r | CapabilityRights::Write;
    }
    if (grant) {
        r = r | CapabilityRights::Grant;
    }
    if (exec) {
        r = r | CapabilityRights::Execute;
    }
    const auto cap = pbsd::kernel::encode(r);
    return cap.cr_rights[0];
}

export extern "C" int pbsd_kernel_cap_check(unsigned long long have_w0,
                                            unsigned long long have_w1,
                                            unsigned long long need_w0,
                                            unsigned long long need_w1) {
    using namespace pbsd::kernel::capsicum;
    const CapRights have = pbsd::kernel::from_cap_rights_words(have_w0, have_w1);
    const CapRights need = pbsd::kernel::from_cap_rights_words(need_w0, need_w1);
    return static_cast<int>(cap_check(have, need));
}

export extern "C" int pbsd_kernel_cap_rights_limit(unsigned long long have_w0,
                                                   unsigned long long have_w1,
                                                   unsigned long long limit_w0,
                                                   unsigned long long limit_w1) {
    using namespace pbsd::kernel::capsicum;
    const CapRights have  = pbsd::kernel::from_cap_rights_words(have_w0, have_w1);
    const CapRights limit = pbsd::kernel::from_cap_rights_words(limit_w0, limit_w1);
    return static_cast<int>(cap_rights_limit(have, limit));
}

export extern "C" unsigned pbsd_kernel_cap_rights_to_vmprot(unsigned long long w0,
                                                           unsigned long long w1) {
    using namespace pbsd::kernel::capsicum;
    const CapRights rights = pbsd::kernel::from_cap_rights_words(w0, w1);
    return cap_rights_to_vmprot(rights);
}

export extern "C" int pbsd_kernel_revoke_lineage(pbsd_lineage_id_t id) {
    pbsd::LineageTree tree{};
    return static_cast<int>(pbsd::kernel::revoke_lineage_hook(tree, id));
}

export extern "C" int pbsd_kernel_validate_wx(unsigned prot) {
    using namespace pbsd::kernel::vm;
    return static_cast<int>(validate_wx_vm(static_cast<unsigned char>(prot)));
}

export extern "C" unsigned pbsd_kernel_prot_max_extract(unsigned encoded) {
    using namespace pbsd::kernel::vm;
    return prot_max_extract(encoded);
}

export extern "C" unsigned pbsd_kernel_merge_entry_offset(unsigned current_prot,
                                                          unsigned max_prot) {
    using namespace pbsd::kernel::vm;
    return merge_entry_offset(static_cast<unsigned char>(current_prot),
                              static_cast<unsigned char>(max_prot));
}

export extern "C" int pbsd_kernel_pax_mprotect_enforce(unsigned old_prot, unsigned new_prot,
                                                     int mprotect_active) {
    using namespace pbsd::kernel::security;
    return static_cast<int>(
        mprotect_enforce(static_cast<unsigned char>(old_prot),
                         static_cast<unsigned char>(new_prot),
                         mprotect_active != 0));
}

export extern "C" unsigned pbsd_kernel_pax_mprotect_apply(unsigned prot, unsigned maxprot) {
    using namespace pbsd::kernel::security;
    return mprotect_apply(static_cast<unsigned char>(prot),
                          static_cast<unsigned char>(maxprot));
}

export extern "C" unsigned pbsd_kernel_pax_pageexec_apply(unsigned prot) {
    using namespace pbsd::kernel::security;
    return pageexec_apply_prot(static_cast<unsigned char>(prot));
}

export extern "C" unsigned long long pbsd_kernel_aslr_delta(unsigned long long seed,
                                                            unsigned lsb,
                                                            unsigned len) {
    using namespace pbsd::kernel::security::aslr;
    return aslr_delta(seed, lsb, len);
}

export extern "C" unsigned long long pbsd_kernel_aslr_apply_mmap(unsigned long long addr,
                                                               unsigned long long orig_addr,
                                                               int mmap_flags,
                                                               unsigned long long delta,
                                                               int active) {
    using namespace pbsd::kernel::security::aslr;
    return apply_mmap_delta(addr, orig_addr, mmap_flags, delta, active != 0);
}

export extern "C" int pbsd_kernel_priority_cap_check(int which, int who, int self_pid,
                                                     int in_cap_mode) {
    using namespace pbsd::kernel::resource;
    return static_cast<int>(
        cap_mode_priority_allowed(which, who, self_pid, in_cap_mode != 0));
}

export extern "C" int pbsd_kernel_p_cansee(unsigned viewer_ruid, unsigned subject_ruid,
                                           unsigned viewer_jail, unsigned subject_jail,
                                           int see_other_uids) {
    using namespace pbsd::kernel::prot;
    BsdVisibilityPolicy pol{};
    pol.see_other_uids = see_other_uids != 0;
    const Ucred viewer{.ruid = viewer_ruid, .jail_id = viewer_jail};
    const Ucred subject{.ruid = subject_ruid, .jail_id = subject_jail};
    return static_cast<int>(p_cansee(viewer, subject, pol));
}

export extern "C" int pbsd_kernel_cap_rights_is_valid(unsigned long long w0,
                                                    unsigned long long w1) {
    using namespace pbsd::kernel::capsicum;
    const CapRights rights = pbsd::kernel::from_cap_rights_words(w0, w1);
    return cap_rights_is_valid(rights) ? static_cast<int>(pbsd::Status::Ok)
                                       : static_cast<int>(pbsd::Status::Invalid);
}

export extern "C" int pbsd_kernel_str2sig(const char* name) {
    using namespace pbsd::kernel::signal;
    const auto r = str2sig(name);
    return r.has_value() ? r.value : -1;
}

export extern "C" int pbsd_kernel_errno_to_status(int errno_val) {
    using namespace pbsd::kernel::errno_ns;
    const auto e = from_errno_int(errno_val);
    if (!e.has_value()) {
        return static_cast<int>(pbsd::Status::Protocol);
    }
    return static_cast<int>(to_status(e.value));
}

export extern "C" int pbsd_kernel_sysctl_top_number(const char* name) {
    using namespace pbsd::kernel::sysctl;
    const auto n = top_level_number(name);
    return n.has_value() ? n.value : -1;
}

export extern "C" int pbsd_kernel_validate_rlimit_id(int which) {
    using namespace pbsd::kernel::resource;
    return static_cast<int>(validate_rlimit_id(which));
}

export extern "C" int pbsd_kernel_pager_status_ok(int pager_code) {
    using namespace pbsd::kernel::vm_pager;
    return pager_ok(pager_code) ? static_cast<int>(pbsd::Status::Ok)
                                : static_cast<int>(pager_status(pager_code));
}

export extern "C" int pbsd_kernel_validate_mib_depth(unsigned depth) {
    using namespace pbsd::kernel::sysctl;
    return static_cast<int>(validate_mib_depth(depth));
}
