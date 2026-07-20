module;
#include <cstddef>

export module pbsd.userland.util.getlocalbase;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libutil/getlocalbase.c
export namespace pbsd::userland::util {

[[nodiscard]] inline const char* getlocalbase_path() noexcept { return "/usr/local"; }

} // namespace pbsd::userland::util
