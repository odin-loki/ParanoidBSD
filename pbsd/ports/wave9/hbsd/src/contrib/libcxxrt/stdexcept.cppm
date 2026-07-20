export module pbsd.port.wave9.hbsd.src.contrib.libcxxrt.stdexcept;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libcxxrt/stdexcept.cc
// void stdexcept_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libcxxrt/stdexcept.cc wave=wave9 loc=99
export namespace pbsd::port::wave9::hbsd::src::contrib::libcxxrt::stdexcept {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libcxxrt::stdexcept
