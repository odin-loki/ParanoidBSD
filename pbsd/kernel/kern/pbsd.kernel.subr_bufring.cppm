module;
#include <cstdint>

export module pbsd.kernel.subr_bufring;

import pbsd.core;
import pbsd.kernel.bufring;

/// Freestanding port of `kern/subr_bufring.c` helpers.
export namespace pbsd::kernel::subr_bufring {

[[nodiscard]] inline Status enqueue_slot(bufring::Ring& br) noexcept {
    const unsigned occ = bufring::prod_occupancy(br);
    if (occ >= bufring::prod_capacity(br)) {
        return Status::Busy;
    }
    ++br.prod_tail;
    return Status::Ok;
}

[[nodiscard]] inline Status dequeue_slot(bufring::Ring& br) noexcept {
    if (br.cons_head == br.cons_tail) {
        return Status::NotFound;
    }
    ++br.cons_head;
    return Status::Ok;
}

} // namespace pbsd::kernel::subr_bufring
