module;
#include <cstdint>

export module pbsd.kernel.sysctl;

import pbsd.core;

/// Freestanding port of `sys/sysctl.h` top-level OID tables + name lookup.
export namespace pbsd::kernel::sysctl {

inline constexpr int kCtlMaxName = 24;
inline constexpr int kOidAuto    = -1;
inline constexpr unsigned kCtlAutoStart = 0x100u;

inline constexpr unsigned kCtlType      = 0xfu;
inline constexpr unsigned kCtlTypeNode  = 1u;
inline constexpr unsigned kCtlTypeInt   = 2u;
inline constexpr unsigned kCtlTypeString = 3u;
inline constexpr unsigned kCtlTypeOpaque = 5u;

inline constexpr unsigned kCtlFlagRd   = 0x80000000u;
inline constexpr unsigned kCtlFlagWr   = 0x40000000u;
inline constexpr unsigned kCtlFlagRw   = kCtlFlagRd | kCtlFlagWr;
inline constexpr unsigned kCtlFlagCapRd = 0x00008000u;
inline constexpr unsigned kCtlFlagCapWr = 0x00004000u;
inline constexpr unsigned kCtlFlagCapRw = kCtlFlagCapRd | kCtlFlagCapWr;

// Top-level identifiers — sysctl.h
inline constexpr int kCtlSysctl  = 0;
inline constexpr int kCtlKern    = 1;
inline constexpr int kCtlVm      = 2;
inline constexpr int kCtlVfs     = 3;
inline constexpr int kCtlNet     = 4;
inline constexpr int kCtlDebug   = 5;
inline constexpr int kCtlHw      = 6;
inline constexpr int kCtlMachdep = 7;
inline constexpr int kCtlUser    = 8;

// CTL_KERN subset
inline constexpr int kKernOstype      = 1;
inline constexpr int kKernOsrelease   = 2;
inline constexpr int kKernMaxproc     = 6;
inline constexpr int kKernMaxfiles    = 7;
inline constexpr int kKernSecurelvl   = 9;
inline constexpr int kKernHostname    = 10;
inline constexpr int kKernProc        = 14;
inline constexpr int kKernMaxfilesperproc = 27;

// CTL_VM subset — vm_param.h
inline constexpr int kVmTotal            = 1;
inline constexpr int kVmLoadavg          = 2;
inline constexpr int kVmVFreeMin         = 3;
inline constexpr int kVmSwappingEnabled  = 11;
inline constexpr int kVmMaxId            = 13;

// CTL_HW subset
inline constexpr int kHwMachine  = 1;
inline constexpr int kHwModel    = 2;
inline constexpr int kHwNcpu     = 3;
inline constexpr int kHwPhysmem  = 5;

// CTL_DEBUG subset
inline constexpr int kDebugName = 1;

// CTL_NET subset (netinet/tcp.h paths)
inline constexpr int kNetRoute   = 0;
inline constexpr int kNetInet      = 2;
inline constexpr int kNetInet6     = 3;

struct TopLevelOid {
    int         number;
    const char* name;
};

inline constexpr TopLevelOid kTopLevelTable[] = {
    {kCtlSysctl,  "sysctl"},
    {kCtlKern,    "kern"},
    {kCtlVm,      "vm"},
    {kCtlVfs,     "vfs"},
    {kCtlNet,     "net"},
    {kCtlDebug,   "debug"},
    {kCtlHw,      "hw"},
    {kCtlMachdep, "machdep"},
    {kCtlUser,    "user"},
};

struct KernOid {
    int         number;
    const char* name;
};

inline constexpr KernOid kKernTable[] = {
    {kKernOstype,      "ostype"},
    {kKernOsrelease,   "osrelease"},
    {kKernMaxproc,     "maxproc"},
    {kKernMaxfiles,    "maxfiles"},
    {kKernSecurelvl,   "securelevel"},
    {kKernHostname,    "hostname"},
    {kKernProc,        "proc"},
    {kKernMaxfilesperproc, "maxfilesperproc"},
};

struct VmOid {
    int         number;
    const char* name;
};

inline constexpr VmOid kVmTable[] = {
    {kVmTotal,           "vmtotal"},
    {kVmLoadavg,         "loadavg"},
    {kVmVFreeMin,        "v_free_min"},
    {kVmSwappingEnabled, "swap_enabled"},
    {kVmMaxId,           "maxid"},
};

struct HwOid {
    int         number;
    const char* name;
};

inline constexpr HwOid kHwTable[] = {
    {kHwMachine, "machine"},
    {kHwModel,   "model"},
    {kHwNcpu,    "ncpu"},
    {kHwPhysmem, "physmem"},
};

[[nodiscard]] inline unsigned kern_table_size() noexcept {
    return static_cast<unsigned>(sizeof(kKernTable) / sizeof(kKernTable[0]));
}

[[nodiscard]] inline unsigned vm_table_size() noexcept {
    return static_cast<unsigned>(sizeof(kVmTable) / sizeof(kVmTable[0]));
}

[[nodiscard]] inline unsigned hw_table_size() noexcept {
    return static_cast<unsigned>(sizeof(kHwTable) / sizeof(kHwTable[0]));
}

[[nodiscard]] inline Result<int> kern_number(const char* name) noexcept {
    if (name == nullptr) {
        return result_err<int>(Status::Invalid);
    }
    for (const auto& e : kKernTable) {
        const char* a = name;
        const char* b = e.name;
        while (*a && *b && *a == *b) {
            ++a;
            ++b;
        }
        if (*a == '\0' && *b == '\0') {
            return result_ok(e.number);
        }
    }
    return result_err<int>(Status::NotFound);
}

[[nodiscard]] inline unsigned top_level_table_size() noexcept {
    return static_cast<unsigned>(sizeof(kTopLevelTable) / sizeof(kTopLevelTable[0]));
}

[[nodiscard]] inline Result<const char*> top_level_name(int n) noexcept {
    for (const auto& e : kTopLevelTable) {
        if (e.number == n) {
            return result_ok(e.name);
        }
    }
    return result_err<const char*>(Status::NotFound);
}

[[nodiscard]] inline Result<int> top_level_number(const char* name) noexcept {
    if (name == nullptr) {
        return result_err<int>(Status::Invalid);
    }
    for (const auto& e : kTopLevelTable) {
        const char* a = name;
        const char* b = e.name;
        while (*a && *b && *a == *b) {
            ++a;
            ++b;
        }
        if (*a == '\0' && *b == '\0') {
            return result_ok(e.number);
        }
    }
    return result_err<int>(Status::NotFound);
}

/// `sysctl_handle_int` cap-mode gate — read-only oids allowed with CTLFLAG_CAPRD.
[[nodiscard]] constexpr Status cap_mode_access(unsigned flags, bool writing,
                                               bool in_cap_mode) noexcept {
    if (!in_cap_mode) {
        return Status::Ok;
    }
    if (writing) {
        return (flags & kCtlFlagCapWr) != 0 ? Status::Ok : Status::Denied;
    }
    return (flags & kCtlFlagCapRd) != 0 ? Status::Ok : Status::Denied;
}

/// Validate MIB path length — `sysctl()` name[] bounds.
[[nodiscard]] constexpr Status validate_mib_depth(unsigned depth) noexcept {
    if (depth == 0 || depth > static_cast<unsigned>(kCtlMaxName)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr unsigned decode_ctltype(unsigned kind) noexcept {
    return kind & kCtlType;
}

[[nodiscard]] constexpr bool is_node(unsigned kind) noexcept {
    return decode_ctltype(kind) == kCtlTypeNode;
}

} // namespace pbsd::kernel::sysctl
