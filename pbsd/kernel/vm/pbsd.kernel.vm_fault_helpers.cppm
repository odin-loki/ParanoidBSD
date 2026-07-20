module;
#include <cstdint>

export module pbsd.kernel.vm_fault_helpers;

import pbsd.core;
import pbsd.kernel.vm_fault;
import pbsd.kernel.vm;

/// Freestanding port of `vm/vm_fault.c`.
export namespace pbsd::kernel::vm_fault_helpers {

using namespace pbsd::kernel::vm_fault;

[[nodiscard]] inline Status validate_fault(FaultState& st, unsigned char prot) noexcept {
    if ((st.max_prot & prot) != prot) {
        return Status::Denied;
    }
    st.fault_type = prot;
    return Status::Ok;
}

[[nodiscard]] inline bool retryable(FaultResult r) noexcept {
    return r == FaultResult::Restart || r == FaultResult::Continue;
}

} // namespace pbsd::kernel::vm_fault_helpers
