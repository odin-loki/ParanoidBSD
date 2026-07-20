module;

#include <cstddef>

export module pbsd.userland.getent;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/getent/getent.c — nss database dispatch table.
export namespace pbsd::userland::usr_bin::getent {

enum class Database : int {
    Ethers,
    Group,
    Hosts,
    Netgroup,
    Networks,
    Passwd,
    Protocols,
    Rpc,
    Services,
    Shells,
    Utmpx,
    Unknown,
};

struct DatabaseEntry {
    const char* name;
    Database id;
};

inline constexpr DatabaseEntry kDatabases[] = {
    {"ethers", Database::Ethers},     {"group", Database::Group},
    {"hosts", Database::Hosts},       {"netgroup", Database::Netgroup},
    {"networks", Database::Networks},   {"passwd", Database::Passwd},
    {"protocols", Database::Protocols}, {"rpc", Database::Rpc},
    {"services", Database::Services},   {"shells", Database::Shells},
    {"utmpx", Database::Utmpx},
};

[[nodiscard]] inline Database lookup_database(const char* name) noexcept {
    if (name == nullptr) {
        return Database::Unknown;
    }
    for (const auto& e : kDatabases) {
        if (hosted::cstrcmp(name, e.name) == 0) {
            return e.id;
        }
    }
    return Database::Unknown;
}

[[nodiscard]] inline const char* database_name(Database db) noexcept {
    for (const auto& e : kDatabases) {
        if (e.id == db) {
            return e.name;
        }
    }
    return nullptr;
}

} // namespace pbsd::userland::usr_bin::getent
