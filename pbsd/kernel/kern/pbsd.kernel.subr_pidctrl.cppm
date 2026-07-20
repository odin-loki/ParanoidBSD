module;

export module pbsd.kernel.subr_pidctrl;

import pbsd.core;

/// Freestanding port of `kern/subr_pidctrl.c` — PID controller math helpers.
export namespace pbsd::kernel::subr_pidctrl {

[[nodiscard]] inline int compute_error(int setpoint, int input) noexcept {
    return setpoint - input;
}

[[nodiscard]] inline Status validate_gains(int kpd, int kid, int kdd) noexcept {
    if (kpd <= 0 || kid <= 0 || kdd <= 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline int clamp_integral(int integral, int bound) noexcept {
    if (integral > bound) {
        return bound;
    }
    if (integral < -bound) {
        return -bound;
    }
    return integral;
}

} // namespace pbsd::kernel::subr_pidctrl
