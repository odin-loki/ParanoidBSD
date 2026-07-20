module;
#include <cstdint>

export module pbsd.kernel.subr_vmem;

import pbsd.core;
import pbsd.kernel.vmem;

/// PROVENANCE: hbsd/src/sys/kern/subr_vmem.c — vmem(9) fit/alloc subr.
export namespace pbsd::kernel::subr_vmem {

[[nodiscard]] inline Status alloc_range(std::uint64_t start,
                                        std::uint64_t size) noexcept {
    return vmem::validate_range(start, size);
}

[[nodiscard]] inline bool fit_first(unsigned flags) noexcept {
    return (flags & static_cast<unsigned>(vmem::FitFlag::First)) != 0;
}

[[nodiscard]] inline bool fit_best(unsigned flags) noexcept {
    return (flags & static_cast<unsigned>(vmem::FitFlag::Best)) != 0;
}

} // namespace pbsd::kernel::subr_vmem
