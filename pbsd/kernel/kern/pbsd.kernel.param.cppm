module;
#include <cstdint>

export module pbsd.kernel.param;

import pbsd.core;

/// Freestanding port of `subr_param.c` tunable formulae + clock constants.
export namespace pbsd::kernel::param {

inline constexpr int kDefaultHz    = 1000;
inline constexpr int kDefaultHzVm  = 100;

[[nodiscard]] constexpr int nproc_from_maxusers(int maxusers) noexcept {
    return 20 + 16 * maxusers;
}

[[nodiscard]] constexpr int maxfiles_from_maxusers(int maxusers) noexcept {
    return 40 + 32 * maxusers;
}

[[nodiscard]] constexpr int tick_usec(int hz) noexcept {
    if (hz <= 0) {
        return 0;
    }
    return 1000000 / hz;
}

struct KernelTunables {
    int hz{kDefaultHz};
    int hz_vm{kDefaultHzVm};
    int maxusers{0};
    int maxproc{};
    int maxfiles{};
    int maxfilesperproc{};
    int maxprocperuid{};
};

[[nodiscard]] inline KernelTunables init_param2(int maxusers) noexcept {
    KernelTunables t{};
    t.maxusers = maxusers;
    t.maxproc = nproc_from_maxusers(maxusers);
    t.maxfiles = maxfiles_from_maxusers(maxusers);
    t.maxfilesperproc = t.maxfiles > 0 ? t.maxfiles / 4 : 0;
    t.maxprocperuid = t.maxproc > 0 ? t.maxproc / 2 : 0;
    return t;
}

[[nodiscard]] constexpr Status validate_maxusers(int maxusers) noexcept {
    if (maxusers < 0 || maxusers > 4096) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::param
