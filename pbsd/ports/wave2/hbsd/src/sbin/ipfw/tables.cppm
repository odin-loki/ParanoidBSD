export module pbsd.port.wave2.hbsd.src.sbin.ipfw.tables;

module;
extern "C" {
// Legacy TU entry — hand-port algorithm from hbsd/src/sbin/ipfw/tables.c
int main(int argc, char* argv[]);
}

export import pbsd.userland.capsicum.helpers;
import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sbin/ipfw/tables.c wave=wave2 loc=2096
export namespace pbsd::port::wave2::hbsd::src::sbin::ipfw::tables {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::sbin::ipfw::tables
