export module pbsd.port.wave2.hbsd.src.bin.ln.ln;

module;
extern "C" {
// Legacy TU entry — hand-port algorithm from hbsd/src/bin/ln/ln.c
int main(int argc, char* argv[]);
}

export import pbsd.userland.capsicum.helpers;
import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/bin/ln/ln.c wave=wave2 loc=378
export namespace pbsd::port::wave2::hbsd::src::bin::ln::ln {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::bin::ln::ln
