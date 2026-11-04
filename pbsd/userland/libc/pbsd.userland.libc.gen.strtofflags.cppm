module;
#include <cstdint>

export module pbsd.userland.libc.gen.strtofflags;

export import pbsd.core;

/// strtofflags from hbsd/src/lib/libc/gen/strtofflags.c
export namespace pbsd::userland::libc {

inline constexpr unsigned kSfAppend = 0x00040000u;
inline constexpr unsigned kSfImmutable = 0x00020000u;
inline constexpr unsigned kUfAppend = 0x00000004u;
inline constexpr unsigned kUfImmutable = 0x00000002u;
inline constexpr unsigned kUfNodump = 0x00000001u;

struct FlagMapping {
    const char* name;
    bool invert;
    unsigned flag;
};

[[nodiscard]] inline unsigned lookup_flag(FlagMapping const* table, unsigned n,
                                          char const* name, bool& invert_out) noexcept {
    if (table == nullptr || name == nullptr) {
        return 0;
    }
    for (unsigned i = 0; i < n; ++i) {
        bool match = true;
        for (unsigned j = 0; name[j] != '\0' || table[i].name[j] != '\0'; ++j) {
            if (name[j] != table[i].name[j]) {
                match = false;
                break;
            }
        }
        if (match) {
            invert_out = table[i].invert;
            return table[i].flag;
        }
    }
    return 0;
}

[[nodiscard]] inline Status apply_invert(unsigned flag, bool invert,
                                         unsigned& set, unsigned& clear) noexcept {
    if (invert) {
        clear |= flag;
    } else {
        set |= flag;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
