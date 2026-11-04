module;
#include <cstdint>

export module pbsd.pkg.repo;

import pbsd.core;

/// Wave 9 — package repository metadata.
export namespace pbsd::pkg::repo {

enum class RepoFlag : unsigned int {
    Signed    = 0x01,
    Mirror    = 0x02,
    Local     = 0x04,
    Immutable = 0x08,
};

struct RepoRef {
    char name[64]{};
    char url[256]{};
    char fingerprint_hex[65]{};
    unsigned flags{};
};

[[nodiscard]] inline Status validate_repo(RepoRef const& r) noexcept {
    if (r.name[0] == '\0' || r.url[0] == '\0') {
        return Status::Invalid;
    }
    if ((r.flags & static_cast<unsigned>(RepoFlag::Signed))
        && r.fingerprint_hex[0] == '\0') {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::pkg::repo
