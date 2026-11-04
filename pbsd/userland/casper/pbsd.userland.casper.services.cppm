module;

#include <cstring>

export module pbsd.userland.casper.services;

export import pbsd.userland.casper;
export import pbsd.userland.casper.sysctl;
export import pbsd.userland.casper.pwd;
export import pbsd.userland.casper.dns;
export import pbsd.userland.casper.net;
export import pbsd.userland.casper.grp;
export import pbsd.userland.casper.syslog;
export import pbsd.userland.casper.netdb;
export import pbsd.userland.casper.fileargs;
import pbsd.core;

/// Known libcasper service registry from hbsd/src/lib/libcasper/services/*/
export namespace pbsd::userland::casper::services {

inline constexpr const char* kKnownServices[] = {
    sysctl::kServiceName,
    pwd::kServiceName,
    dns::kServiceName,
    net::kServiceName,
    grp::kServiceName,
    syslog::kServiceName,
    netdb::kServiceName,
    fileargs::kServiceName,
};

inline constexpr unsigned kKnownServiceCount =
    sizeof(kKnownServices) / sizeof(kKnownServices[0]);

[[nodiscard]] inline bool is_known_service(const char* name) noexcept {
    if (name == nullptr) {
        return false;
    }
    for (unsigned i = 0; i < kKnownServiceCount; ++i) {
        if (std::strcmp(name, kKnownServices[i]) == 0) {
            return true;
        }
    }
    return false;
}

[[nodiscard]] inline Status validate_known_service(const char* name) noexcept {
    if (validate_service_name(name) != Status::Ok) {
        return Status::Invalid;
    }
    if (!is_known_service(name)) {
        return Status::NotFound;
    }
    return Status::Ok;
}

[[nodiscard]] inline Result<ChannelHandle>
open_known_service(LineageTree& tree, CapabilityRights rights,
                   const char* service) noexcept {
    const auto st = validate_known_service(service);
    if (st != Status::Ok) {
        return {st, ChannelHandle{}};
    }
    return open_service(tree, rights, service);
}

} // namespace pbsd::userland::casper::services
