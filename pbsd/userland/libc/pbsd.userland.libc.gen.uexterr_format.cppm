module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.uexterr_format;

export import pbsd.core;

/// uexterr_format from hbsd/src/lib/libc/gen/uexterr_format.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline const char* uexterr_format_msg(int err) noexcept { (void)err; return "error"; }

} // namespace pbsd::userland::libc
