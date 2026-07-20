export module pbsd.port.wave2.hbsd.src.lib.libdevdctl.exception;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libdevdctl/exception.cc
// void exception_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libdevdctl/exception.cc wave=wave2 loc=122
export namespace pbsd::port::wave2::hbsd::src::lib::libdevdctl::exception {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libdevdctl::exception
