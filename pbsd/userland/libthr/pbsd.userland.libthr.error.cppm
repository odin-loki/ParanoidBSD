module;
#include <cstdint>

export module pbsd.userland.libthr.error;

import pbsd.core;

/// pthread error-code mapping scaffold from hbsd/src/lib/libthr
export namespace pbsd::userland::libthr {

enum class ThrError : int {
    Ok = 0,
    Inval = 22,
    Again = 35,
    Deadlk = 11,
    Perm = 1,
    Srch = 3,
    Busy = 16,
    TimedOut = 60,
};

[[nodiscard]] inline Status thr_error_to_status(ThrError e) noexcept {
    switch (e) {
    case ThrError::Ok:
        return Status::Ok;
    case ThrError::Inval:
        return Status::Invalid;
    case ThrError::Again:
    case ThrError::Busy:
        return Status::Busy;
    case ThrError::TimedOut:
        return Status::Timeout;
    case ThrError::Perm:
        return Status::Denied;
    case ThrError::Srch:
        return Status::NotFound;
    case ThrError::Deadlk:
        return Status::Protocol;
    }
    return Status::Invalid;
}

} // namespace pbsd::userland::libthr
