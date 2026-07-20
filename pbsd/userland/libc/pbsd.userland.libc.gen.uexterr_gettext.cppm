module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.uexterr_gettext;

export import pbsd.core;

/// uexterr_gettext from hbsd/src/lib/libc/gen/uexterr_gettext.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline const char* uexterr_gettext_msg(int err) noexcept { (void)err; return "error"; }

} // namespace pbsd::userland::libc
