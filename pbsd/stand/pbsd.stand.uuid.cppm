module;
#include <cstddef>
#include <cstdint>

export module pbsd.stand.uuid;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/libsa/uuid.c
export namespace pbsd::stand::uuid {

inline constexpr unsigned kUuidLen = 36;

[[nodiscard]] inline Status validate_uuid_len(unsigned len) noexcept {
    return len == kUuidLen ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::stand::uuid
