module;
#include <cstdint>

export module pbsd.kernel.subr_dummy_vdso_tc;

export import pbsd.core;

/// Freestanding port of `kern/subr_dummy_vdso_tc.c` — dummy vdso tc helpers.
export namespace pbsd::kernel::subr_dummy_vdso_tc {

inline constexpr unsigned kVdsoGen = 0;

[[nodiscard]] inline Status publish(unsigned& gen) noexcept {
    ++gen;
    return Status::Ok;
}

} // namespace pbsd::kernel::subr_dummy_vdso_tc
