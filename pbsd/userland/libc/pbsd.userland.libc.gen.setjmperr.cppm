module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.setjmperr;

export import pbsd.core;

/// setjmperr from hbsd/src/lib/libc/gen/setjmperr.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status setjmperr_code(int code) noexcept { return code == 0 ? Status::Ok : Status::Invalid; }

} // namespace pbsd::userland::libc
