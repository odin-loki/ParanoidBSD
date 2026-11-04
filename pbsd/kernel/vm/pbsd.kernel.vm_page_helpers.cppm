module;
#include <cstdint>

export module pbsd.kernel.vm_page_helpers;

import pbsd.core;
import pbsd.kernel.vm_page;

/// Freestanding port of `vm/vm_page.c`.
export namespace pbsd::kernel::vm_page_helpers {

using namespace pbsd::kernel::vm_page;

[[nodiscard]] inline Status move_queue(PageQueueEntry& e, unsigned new_q) noexcept {
    if (new_q >= kPqCount && new_q != kPqNone) {
        return Status::Invalid;
    }
    e.queue = new_q;
    return Status::Ok;
}

[[nodiscard]] inline bool laundry_candidate(unsigned q) noexcept {
    return is_inactive_queue(q) || is_laundry_family(q);
}

} // namespace pbsd::kernel::vm_page_helpers
