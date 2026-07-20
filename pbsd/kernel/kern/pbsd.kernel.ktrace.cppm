module;
#include <cstdint>

export module pbsd.kernel.ktrace;

import pbsd.core;

/// Freestanding port of `sys/ktrace.h` operations, record types, and trace flags.
export namespace pbsd::kernel::ktrace {

inline constexpr int kKtropSet       = 0;
inline constexpr int kKtropClear     = 1;
inline constexpr int kKtropClearfile = 2;
inline constexpr int kKtrflagDescend = 4;

inline constexpr int kKtrVersion0 = 0;
inline constexpr int kKtrVersion1 = 1;

inline constexpr int kKtrSyscall     = 1;
inline constexpr int kKtrSysret      = 2;
inline constexpr int kKtrNamei       = 3;
inline constexpr int kKtrGenio       = 4;
inline constexpr int kKtrPsig        = 5;
inline constexpr int kKtrCsw         = 6;
inline constexpr int kKtrUser        = 7;
inline constexpr int kKtrStruct      = 8;
inline constexpr int kKtrSysctl      = 9;
inline constexpr int kKtrProcctor    = 10;
inline constexpr int kKtrProcdtor    = 11;
inline constexpr int kKtrCapfail     = 12;
inline constexpr int kKtrFault       = 13;
inline constexpr int kKtrFaultend    = 14;
inline constexpr int kKtrStructArray = 15;
inline constexpr int kKtrArgs        = 16;
inline constexpr int kKtrEnvs        = 17;
inline constexpr int kKtrExterr      = 18;

inline constexpr unsigned kKtrDrop      = 0x8000u;
inline constexpr unsigned kKtrVersioned = 0x4000u;
inline constexpr unsigned kKtrTypeMask  = kKtrDrop | kKtrVersioned;

inline constexpr unsigned kKtrfacMask       = 0x00ffffffu;
inline constexpr unsigned kKtrfacSyscall    = 1u << kKtrSyscall;
inline constexpr unsigned kKtrfacSysret       = 1u << kKtrSysret;
inline constexpr unsigned kKtrfacNamei       = 1u << kKtrNamei;
inline constexpr unsigned kKtrfacGenio       = 1u << kKtrGenio;
inline constexpr unsigned kKtrfacPsig        = 1u << kKtrPsig;
inline constexpr unsigned kKtrfacCsw         = 1u << kKtrCsw;
inline constexpr unsigned kKtrfacUser        = 1u << kKtrUser;
inline constexpr unsigned kKtrfacCapfail     = 1u << kKtrCapfail;
inline constexpr unsigned kKtrfacRoot        = 0x80000000u;
inline constexpr unsigned kKtrfacInherit     = 0x40000000u;
inline constexpr unsigned kKtrfacDrop        = 0x20000000u;

[[nodiscard]] constexpr int ktrop(int op) noexcept {
    return op & 3;
}

[[nodiscard]] constexpr bool is_descend(unsigned op) noexcept {
    return (static_cast<unsigned>(op) & kKtrflagDescend) != 0;
}

[[nodiscard]] constexpr bool ktrcheck(unsigned traceflag, int type) noexcept {
    if (type < 0 || type > 31) {
        return false;
    }
    return (traceflag & (1u << type)) != 0;
}

[[nodiscard]] constexpr unsigned enable_facility(unsigned traceflag, int type) noexcept {
    if (type < 0 || type > 31) {
        return traceflag;
    }
    return traceflag | (1u << type);
}

[[nodiscard]] constexpr unsigned disable_facility(unsigned traceflag, int type) noexcept {
    if (type < 0 || type > 31) {
        return traceflag;
    }
    return traceflag & ~(1u << type);
}

struct KtraceTypeEntry {
    int         type;
    const char* name;
};

inline constexpr KtraceTypeEntry kTypeTable[] = {
    {kKtrSyscall,  "syscall"},
    {kKtrSysret,   "sysret"},
    {kKtrNamei,    "namei"},
    {kKtrGenio,    "genio"},
    {kKtrPsig,     "psig"},
    {kKtrCsw,      "csw"},
    {kKtrCapfail,  "capfail"},
    {kKtrFault,    "fault"},
};

[[nodiscard]] inline unsigned type_table_size() noexcept {
    return static_cast<unsigned>(sizeof(kTypeTable) / sizeof(kTypeTable[0]));
}

} // namespace pbsd::kernel::ktrace
