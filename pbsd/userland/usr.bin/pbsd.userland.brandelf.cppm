module;
#include <cstddef>

export module pbsd.userland.brandelf;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/brandelf/brandelf.c — ELF ABI branding (logic-only).
export namespace pbsd::userland::usr_bin::brandelf {

inline constexpr unsigned char kElfMag0 = 0x7f;
inline constexpr unsigned char kElfMag1 = 'E';
inline constexpr unsigned char kElfMag2 = 'L';
inline constexpr unsigned char kElfMag3 = 'F';
inline constexpr int kOsabiFreebsd = 9;
inline constexpr int kOsabiLinux = 3;
inline constexpr int kOsabiSolaris = 6;
inline constexpr int kOsabiSysv = 0;

struct ElfAbi {
    const char* name;
    int value;
};

inline constexpr ElfAbi kElfTypes[] = {
    {"FreeBSD", kOsabiFreebsd},
    {"Linux", kOsabiLinux},
    {"Solaris", kOsabiSolaris},
    {"SVR4", kOsabiSysv},
};

[[nodiscard]] inline bool is_elf_magic(const unsigned char ident[4]) noexcept {
    return ident != nullptr && ident[0] == kElfMag0 && ident[1] == kElfMag1 &&
           ident[2] == kElfMag2 && ident[3] == kElfMag3;
}

[[nodiscard]] inline const char* abi_name(int etype) noexcept {
    for (const ElfAbi& e : kElfTypes) {
        if (e.value == etype) {
            return e.name;
        }
    }
    return nullptr;
}

[[nodiscard]] inline Result<int> parse_abi_string(const char* str) noexcept {
    if (str == nullptr) {
        return result_err<int>(Status::Invalid);
    }
    for (const ElfAbi& e : kElfTypes) {
        if (hosted::cstrcmp(str, e.name) == 0) {
            return result_ok(e.value);
        }
    }
    return result_err<int>(Status::Invalid);
}

} // namespace pbsd::userland::usr_bin::brandelf
