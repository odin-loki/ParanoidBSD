export module pbsd.pkg;

import pbsd.core;
export import pbsd.pkg.manifest;
export import pbsd.pkg.repo;
export import pbsd.pkg.signature;
export import pbsd.pkg.index;
export import pbsd.pkg.bootstrap;
export import pbsd.pkg.cache;
export import pbsd.pkg.lockfile;
export import pbsd.pkg.resolver;
export import pbsd.pkg.deps;
export import pbsd.pkg.verify;
export import pbsd.pkg.transaction;

/// Wave 9 — atomic package update skeleton (content-addressed transactions).
export namespace pbsd::pkg {

enum class TxState : unsigned char {
    Idle = 0,
    Staging = 1,
    Applied = 2,
    RolledBack = 3,
};

struct PackageRef {
    char name[64]{};
    char digest_hex[65]{}; // sha256 placeholder
};

struct Transaction {
    TxState state{TxState::Idle};
    unsigned package_count{0};
    PackageRef packages[16]{};
};

class AtomicUpdater {
public:
    [[nodiscard]] Status begin(Transaction& tx) noexcept {
        if (tx.state != TxState::Idle) {
            return Status::Busy;
        }
        tx.state = TxState::Staging;
        tx.package_count = 0;
        return Status::Ok;
    }

    [[nodiscard]] Status stage(Transaction& tx, PackageRef const& pkg) noexcept {
        if (tx.state != TxState::Staging) {
            return Status::Invalid;
        }
        if (tx.package_count >= 16 || pkg.name[0] == '\0') {
            return Status::Invalid;
        }
        tx.packages[tx.package_count++] = pkg;
        return Status::Ok;
    }

    [[nodiscard]] Status commit(Transaction& tx) noexcept {
        if (tx.state != TxState::Staging || tx.package_count == 0) {
            return Status::Invalid;
        }
        // Scaffold: real path snapshots ZFS datasets + swaps boot env symlink.
        tx.state = TxState::Applied;
        return Status::Ok;
    }

    [[nodiscard]] Status rollback(Transaction& tx) noexcept {
        if (tx.state != TxState::Applied) {
            return Status::Invalid;
        }
        tx.state = TxState::RolledBack;
        return Status::Ok;
    }
};

} // namespace pbsd::pkg
