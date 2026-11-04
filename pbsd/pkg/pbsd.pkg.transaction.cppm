module;
#include <cstdint>

export module pbsd.pkg.transaction;

import pbsd.core;
import pbsd.pkg.manifest;

/// Burst 13 — atomic package transaction staging (content-addressed).
export namespace pbsd::pkg::transaction {

enum class Phase : unsigned char {
    Idle = 0,
    Staging,
    Verified,
    Applied,
    RolledBack,
};

inline constexpr unsigned kMaxPackages = 32;

struct StagedPackage {
    char name[64]{};
    char digest_hex[65]{};
    manifest::EntryType kind{manifest::EntryType::File};
};

struct Tx {
    Phase phase{Phase::Idle};
    unsigned count{0};
    StagedPackage packages[kMaxPackages]{};
};

[[nodiscard]] inline Status begin(Tx& tx) noexcept {
    if (tx.phase != Phase::Idle) {
        return Status::Busy;
    }
    tx.phase = Phase::Staging;
    tx.count = 0;
    return Status::Ok;
}

[[nodiscard]] inline Status stage(Tx& tx, StagedPackage const& pkg) noexcept {
    if (tx.phase != Phase::Staging || tx.count >= kMaxPackages) {
        return Status::Invalid;
    }
    if (pkg.name[0] == '\0' || pkg.digest_hex[0] == '\0') {
        return Status::Invalid;
    }
    tx.packages[tx.count++] = pkg;
    return Status::Ok;
}

[[nodiscard]] inline Status mark_verified(Tx& tx) noexcept {
    if (tx.phase != Phase::Staging || tx.count == 0) {
        return Status::Invalid;
    }
    tx.phase = Phase::Verified;
    return Status::Ok;
}

[[nodiscard]] inline Status commit(Tx& tx) noexcept {
    if (tx.phase != Phase::Verified) {
        return Status::Invalid;
    }
    tx.phase = Phase::Applied;
    return Status::Ok;
}

[[nodiscard]] inline Status rollback(Tx& tx) noexcept {
    if (tx.phase != Phase::Applied) {
        return Status::Invalid;
    }
    tx.phase = Phase::RolledBack;
    return Status::Ok;
}

} // namespace pbsd::pkg::transaction
