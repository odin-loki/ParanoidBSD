module;
#include <cstddef>

export module pbsd.userland.libc.gen.getloadavg;

export import pbsd.core;

/// getloadavg scaffold from hbsd/src/lib/libc/gen/getloadavg.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline StatusOnly getloadavg(double* loadavg, int nelem) noexcept {
    if (loadavg == nullptr || nelem <= 0) {
        return status_err(Status::Invalid);
    }
    const int n = nelem > 3 ? 3 : nelem;
    for (int i = 0; i < n; ++i) {
        loadavg[i] = 0.0;
    }
    return status_err(Status::NotImplemented);
}

} // namespace pbsd::userland::libc
