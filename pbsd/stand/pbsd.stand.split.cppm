module;
#include <cstddef>
#include <cstdint>

export module pbsd.stand.split;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/libsa/split.c
export namespace pbsd::stand::split {

[[nodiscard]] inline Status validate_part_count(unsigned parts) noexcept {
    return parts > 0 && parts <= 64 ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::stand::split
