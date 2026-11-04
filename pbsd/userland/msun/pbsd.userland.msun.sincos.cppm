module;
#include <cstddef>

export module pbsd.userland.msun.sincos;

export import pbsd.core;

/// scaffold from hbsd/src/lib/msun/src/s_sincos.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline StatusOnly sincos_vals(double x, double* s, double* c) noexcept { (void)x; if (s) *s = 0.0; if (c) *c = 1.0; return status_err(Status::NotImplemented); }

} // namespace pbsd::userland::msun
