module;
#include <cstdint>

export module pbsd.bifrost.unrestricted_guest;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/intel/unrestricted_guest.c
export namespace pbsd::bifrost::unrestricted_guest {

[[nodiscard]] inline bool allow_real_mode(unsigned ctrl) noexcept {
    return (ctrl & (1u << 16)) != 0;
}

} // namespace pbsd::bifrost::unrestricted_guest
