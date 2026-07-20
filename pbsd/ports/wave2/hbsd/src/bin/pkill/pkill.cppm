export module pbsd.port.wave2.hbsd.src.bin.pkill.pkill;

module;
extern "C" {
// Legacy TU entry — hand-port algorithm from hbsd/src/bin/pkill/pkill.c
int main(int argc, char* argv[]);
}

export import pbsd.userland.capsicum.helpers;
import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/bin/pkill/pkill.c wave=wave2 loc=874
export namespace pbsd::port::wave2::hbsd::src::bin::pkill::pkill {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::bin::pkill::pkill
