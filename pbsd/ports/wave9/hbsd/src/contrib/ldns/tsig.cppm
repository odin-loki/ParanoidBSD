export module pbsd.port.wave9.hbsd.src.contrib.ldns.tsig;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ldns/tsig.c
// void tsig_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ldns/tsig.c wave=wave9 loc=494
export namespace pbsd::port::wave9::hbsd::src::contrib::ldns::tsig {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ldns::tsig
