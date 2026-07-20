export module pbsd.port.wave2.hbsd.src.lib.libfetch.http;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libfetch/http.c
// void http_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libfetch/http.c wave=wave2 loc=2148
export namespace pbsd::port::wave2::hbsd::src::lib::libfetch::http {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libfetch::http
