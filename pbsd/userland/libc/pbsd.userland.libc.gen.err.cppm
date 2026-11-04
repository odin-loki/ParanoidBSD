module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.err;

export import pbsd.core;

/// err from hbsd/src/lib/libc/gen/err.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int err_exit_code(int eval) noexcept {
    return eval != 0 ? eval : 1;
}

} // namespace pbsd::userland::libc
