export module pbsd.port.wave2.hbsd.src.usr_bin.newgrp.newgrp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/newgrp/newgrp.c
// void newgrp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/newgrp/newgrp.c wave=wave2 loc=313
export namespace pbsd::port::wave2::hbsd::src::usr_bin::newgrp::newgrp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::newgrp::newgrp
