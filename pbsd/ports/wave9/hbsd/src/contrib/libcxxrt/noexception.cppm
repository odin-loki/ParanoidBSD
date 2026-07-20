export module pbsd.port.wave9.hbsd.src.contrib.libcxxrt.noexception;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libcxxrt/noexception.cc
// void noexception_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libcxxrt/noexception.cc wave=wave9 loc=45
export namespace pbsd::port::wave9::hbsd::src::contrib::libcxxrt::noexception {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libcxxrt::noexception
