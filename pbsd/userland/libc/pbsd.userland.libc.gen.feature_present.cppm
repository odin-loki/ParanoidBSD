module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.feature_present;

export import pbsd.core;

/// feature_present from hbsd/src/lib/libc/gen/feature_present.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline bool feature_present_name(const char* feature) noexcept {
    return feature != nullptr && feature[0] != '\0';
}

} // namespace pbsd::userland::libc
