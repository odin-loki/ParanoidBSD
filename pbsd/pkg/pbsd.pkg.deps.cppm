module;

export module pbsd.pkg.deps;

import pbsd.core;

/// Burst 13 — package dependency edge list (name/version pairs).
export namespace pbsd::pkg::deps {

inline constexpr unsigned kMaxDeps = 64;

struct Dependency {
    char package[64]{};
    char version[32]{};
    bool optional{false};
};

struct DepList {
    unsigned count{0};
    Dependency items[kMaxDeps]{};
};

[[nodiscard]] inline Status add(DepList& list, Dependency const& dep) noexcept {
    if (list.count >= kMaxDeps || dep.package[0] == '\0') {
        return Status::Invalid;
    }
    list.items[list.count++] = dep;
    return Status::Ok;
}

[[nodiscard]] inline bool satisfies(DepList const& list, char const* name) noexcept {
    if (name == nullptr) {
        return false;
    }
    for (unsigned i = 0; i < list.count; ++i) {
        bool match = true;
        for (unsigned j = 0; name[j] != '\0' || list.items[i].package[j] != '\0'; ++j) {
            if (name[j] != list.items[i].package[j]) {
                match = false;
                break;
            }
        }
        if (match) {
            return true;
        }
    }
    return false;
}

} // namespace pbsd::pkg::deps
