module;

export module pbsd.kernel.kern_mib;

export import pbsd.core;
import pbsd.kernel.sysctl;

/// Freestanding port of `kern/kern_mib.c` — CTL_KERN OID subset.
export namespace pbsd::kernel::kern_mib {

inline constexpr int kKernOstype = 1;
inline constexpr int kKernOsrelease = 2;
inline constexpr int kKernVersion = 3;
inline constexpr int kKernMaxvnodes = 5;
inline constexpr int kKernMaxproc = 6;
inline constexpr int kKernMaxfiles = 7;
inline constexpr int kKernArgmax = 8;
inline constexpr int kKernSecurelevel = 9;
inline constexpr int kKernHostname = 10;
inline constexpr int kKernHostid = 11;
inline constexpr int kKernClockrate = 12;
inline constexpr int kKernProc = 14;

struct KernOid {
    int number;
    const char* name;
};

inline constexpr KernOid kTable[] = {
    {kKernOstype, "ostype"},
    {kKernOsrelease, "osrelease"},
    {kKernVersion, "version"},
    {kKernMaxproc, "maxproc"},
    {kKernMaxfiles, "maxfiles"},
    {kKernHostname, "hostname"},
    {kKernProc, "proc"},
};

[[nodiscard]] inline const char* lookup_name(int num) noexcept {
    for (const auto& e : kTable) {
        if (e.number == num) {
            return e.name;
        }
    }
    return nullptr;
}

} // namespace pbsd::kernel::kern_mib
