module;

export module pbsd.kernel.pidctrl;

export import pbsd.core;

/// Freestanding port of `sys/pidctrl.h` / `kern/subr_pidctrl.c`.
export namespace pbsd::kernel::pidctrl {

inline constexpr int kDefaultKpd = 3;
inline constexpr int kDefaultKid = 4;
inline constexpr int kDefaultKdd = 8;
inline constexpr int kDefaultBound = 4;

struct Softc {
    int error{};
    int olderror{};
    int integral{};
    int derivative{};
    int input{};
    int output{};
    int ticks{};
    int setpoint{};
    int interval{};
    int bound{};
    int kpd{};
    int kid{};
    int kdd{};
};

[[nodiscard]] inline Status init(Softc& pc, int interval, int setpoint, int bound,
                                 int kpd, int kid, int kdd) noexcept {
    if (interval <= 0 || setpoint <= 0 || kpd <= 0 || kid <= 0 || kdd <= 0) {
        return Status::Invalid;
    }
    pc = {};
    pc.setpoint = setpoint;
    pc.interval = interval;
    pc.bound = bound * setpoint * kid;
    pc.kpd = kpd;
    pc.kid = kid;
    pc.kdd = kdd;
    return Status::Ok;
}

[[nodiscard]] inline int classic_error(const Softc& pc, int input) noexcept {
    return pc.setpoint - input;
}

} // namespace pbsd::kernel::pidctrl
