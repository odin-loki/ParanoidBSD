module;
#include <cstddef>

export module pbsd.userland.util.login_cap;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libutil/login_cap.c
export namespace pbsd::userland::util {

[[nodiscard]] inline StatusOnly login_cap_open(const char* login_class) noexcept {
    if (login_class == nullptr) {
        return status_err(Status::Invalid);
    }
    return status_err(Status::NotImplemented);
}

} // namespace pbsd::userland::util
