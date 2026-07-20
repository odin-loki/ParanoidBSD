export module pbsd.port.wave2.hbsd.src.bin.cp.cp;

module;
extern "C" {
// Legacy TU entry — hand-port algorithm from hbsd/src/bin/cp/cp.c
int main(int argc, char* argv[]);
}

export import pbsd.userland.capsicum.helpers;
import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/bin/cp/cp.c wave=wave2 loc=700
export namespace pbsd::port::wave2::hbsd::src::bin::cp::cp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::bin::cp::cp
