module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.getosreldate;

export import pbsd.core;

/// getosreldate from hbsd/src/lib/libc/gen/getosreldate.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int getosreldate_value() noexcept { return 0; }

} // namespace pbsd::userland::libc
