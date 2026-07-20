export module pbsd.port.wave4.hbsd.src.sys.rpc.replay;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/rpc/replay.c
// void replay_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/rpc/replay.c wave=wave4 loc=255
export namespace pbsd::port::wave4::hbsd::src::sys::rpc::replay {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::rpc::replay
