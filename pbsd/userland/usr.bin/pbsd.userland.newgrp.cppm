module;

export module pbsd.userland.newgrp;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/newgrp/newgrp.c
export namespace pbsd::userland::usr_bin::newgrp {

[[nodiscard]] inline Status newgrp_name(const char* group) noexcept {
    if (group == nullptr || group[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::usr_bin::newgrp
