module;
#include <cstdint>

export module pbsd.kernel.subr_witness;

import pbsd.core;
import pbsd.kernel.witness;

/// Freestanding port of `kern/subr_witness.c` helpers.
export namespace pbsd::kernel::subr_witness {

[[nodiscard]] inline Status validate_order(const witness::LockObjectStub& held,
                                         const witness::LockObjectStub& want) noexcept {
    return witness::check_order(held, want);
}

[[nodiscard]] inline unsigned known_order_count() noexcept {
    return witness::order_table_size();
}

} // namespace pbsd::kernel::subr_witness
