module;

export module pbsd.pkg.resolver;

import pbsd.core;
import pbsd.pkg.index;
import pbsd.pkg.deps;

/// Burst 13 — resolve package name → index entry with dependency closure.
export namespace pbsd::pkg::resolver {

inline constexpr unsigned kMaxResolved = 32;

struct Resolved {
    unsigned count{0};
    index::IndexEntry entries[kMaxResolved]{};
};

[[nodiscard]] inline Status resolve_one(index::PackageIndex const& idx,
                                        char const* name,
                                        index::IndexEntry& out) noexcept {
    return index::lookup(idx, name, out);
}

[[nodiscard]] inline Status resolve(index::PackageIndex const& idx,
                                    deps::DepList const& need,
                                    Resolved& out) noexcept {
    out.count = 0;
    for (unsigned i = 0; i < need.count; ++i) {
        if (out.count >= kMaxResolved) {
            return Status::Denied;
        }
        index::IndexEntry e{};
        const Status s = index::lookup(idx, need.items[i].package, e);
        if (s != Status::Ok) {
            if (need.items[i].optional) {
                continue;
            }
            return s;
        }
        out.entries[out.count++] = e;
    }
    return out.count > 0 ? Status::Ok : Status::NotFound;
}

} // namespace pbsd::pkg::resolver
