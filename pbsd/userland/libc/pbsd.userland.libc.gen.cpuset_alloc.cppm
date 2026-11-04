module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.cpuset_alloc;

export import pbsd.core;

/// cpuset_alloc from hbsd/src/lib/libc/gen/cpuset_alloc.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Result<std::size_t> cpuset_alloc_size(int level) noexcept {
    if (level < 0) {
        return result_err<std::size_t>(Status::Invalid);
    }
    return result_ok(128uz);
}

} // namespace pbsd::userland::libc
