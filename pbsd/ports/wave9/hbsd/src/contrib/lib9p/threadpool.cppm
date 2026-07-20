export module pbsd.port.wave9.hbsd.src.contrib.lib9p.threadpool;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/lib9p/threadpool.c
// void threadpool_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/lib9p/threadpool.c wave=wave9 loc=422
export namespace pbsd::port::wave9::hbsd::src::contrib::lib9p::threadpool {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::lib9p::threadpool
