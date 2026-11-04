module;
#include <cstdint>

export module pbsd.geom.core;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/geom.h — GEOM topology class flags.
export namespace pbsd::geom::core {

enum class ClassFlag : unsigned {
    Rank0 = 0x0001,
    Rank1 = 0x0002,
    Rank2 = 0x0004,
    Rank3 = 0x0008,
};

struct Class {
    const char* name{};
    unsigned rank{};
    unsigned flags{};
};

[[nodiscard]] inline Status validate_class(const Class& c) noexcept {
    if (c.name == nullptr || c.name[0] == '\0') {
        return Status::Invalid;
    }
    if (c.rank > 3) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned rank_flag(unsigned rank) noexcept {
    return 1u << rank;
}

} // namespace pbsd::geom::core
