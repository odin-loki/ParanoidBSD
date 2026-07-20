export module pbsd.port.wave7.hbsd.src.stand.efi.loader.main;

module;
extern "C" {
// Legacy TU entry — hand-port algorithm from hbsd/src/stand/efi/loader/main.c
int main(int argc, char* argv[]);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/stand/efi/loader/main.c wave=wave7 loc=2025
export namespace pbsd::port::wave7::hbsd::src::stand::efi::loader::main {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::stand::efi::loader::main
