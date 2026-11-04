module;
#include <cstdint>

export module pbsd.bifrost.vmfunc;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/intel/vmfunc.c
export namespace pbsd::bifrost::vmfunc {

enum class Function : unsigned char {
    EptpSwitching = 0,
};

[[nodiscard]] inline Status validate_function(unsigned fn) noexcept {
    return fn <= static_cast<unsigned>(Function::EptpSwitching) ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::bifrost::vmfunc
