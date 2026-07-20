module;
#include <cstddef>

export module pbsd.userland.libc.gen.errlst;

export import pbsd.core;

/// errlst scaffold from hbsd/src/lib/libc/gen/errlst.c
export namespace pbsd::userland::libc::gen {

[[nodiscard]] inline const char* errlst_lookup(int errnum) noexcept {
    (void)errnum;
    return "Unknown error";
}

[[nodiscard]] inline std::size_t errlst_count() noexcept { return 0; }

} // namespace pbsd::userland::libc::gen
