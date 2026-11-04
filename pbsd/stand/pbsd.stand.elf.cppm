module;
#include <cstdint>

export module pbsd.stand.elf;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/efi/include/elf.h — ELF class/type constants.
export namespace pbsd::stand::elf {

enum class Class : unsigned char {
    None = 0,
    Elf32 = 1,
    Elf64 = 2,
};

enum class Data : unsigned char {
    None = 0,
    Lsb  = 1,
    Msb  = 2,
};

enum class Type : unsigned short {
    None   = 0,
    Rel    = 1,
    Exec   = 2,
    Dyn    = 3,
    Core   = 4,
};

enum class Machine : unsigned short {
    None   = 0,
    Amd64  = 62,
    Arm64  = 183,
};

[[nodiscard]] inline Status validate_ehdr(Class c, Machine m) noexcept {
    if (c == Class::Elf64 && m != Machine::Amd64 && m != Machine::Arm64) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::stand::elf
