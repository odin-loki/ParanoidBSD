module;
#include <cstdint>

export module pbsd.stand.modload;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/common/module.c — loader module search/dispatch flags.
export namespace pbsd::stand::modload {

inline constexpr const char kDefaultSearchPath[] = "/boot/kernel;/boot/modules";

enum class DirFlag : unsigned int {
    Removed = 0x0001,
    NoHints = 0x0002,
};

enum class LoadStage : unsigned char {
    File = 0,
    Dependencies = 1,
    Metadata = 2,
};

[[nodiscard]] inline Status validate_dir_flags(unsigned flags) noexcept {
    if (flags & static_cast<unsigned>(DirFlag::Removed)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool wants_hints(unsigned flags) noexcept {
    return (flags & static_cast<unsigned>(DirFlag::NoHints)) == 0;
}

} // namespace pbsd::stand::modload
