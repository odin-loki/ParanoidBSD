module;
#include <cstdint>

export module pbsd.fs.devfs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/devfs/devfs.h — devfs magic and rule identifiers.
export namespace pbsd::fs::devfs {

inline constexpr unsigned kMagic = 0xdb0a087a;
inline constexpr unsigned kMaxRules = 65535;

enum class RuleAction : unsigned char {
    Hide = 0,
    Show = 1,
    Link = 2,
};

enum class RuleFlag : unsigned int {
    Recurse = 0x0001,
    NoLink = 0x0002,
};

struct Rule {
    unsigned id{};
    RuleAction action{RuleAction::Hide};
    unsigned flags{};
    char pattern[64]{};
};

[[nodiscard]] inline Status validate_rule_id(unsigned id) noexcept {
    if (id == 0 || id > kMaxRules) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_rule(Rule const& r) noexcept {
    if (validate_rule_id(r.id) != Status::Ok) {
        return Status::Invalid;
    }
    if (r.pattern[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool rule_matches(Rule const& r, char const* name) noexcept {
    if (name == nullptr || r.pattern[0] == '\0') {
        return false;
    }
    unsigned i = 0;
    while (r.pattern[i] != '\0' && name[i] != '\0') {
        if (r.pattern[i] != name[i]) {
            return false;
        }
        ++i;
    }
    return r.pattern[i] == '\0' && name[i] == '\0';
}

} // namespace pbsd::fs::devfs
