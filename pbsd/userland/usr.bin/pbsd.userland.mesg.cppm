module;

export module pbsd.userland.mesg;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/mesg/mesg.c — terminal write permission.
export namespace pbsd::userland::usr_bin::mesg {

enum class Permission : unsigned char {
    Deny,
    Allow,
    Query,
};

[[nodiscard]] inline Permission parse_permission(char c) noexcept {
    switch (c) {
    case 'y':
    case 'Y':
        return Permission::Allow;
    case 'n':
    case 'N':
        return Permission::Deny;
    default:
        return Permission::Query;
    }
}

[[nodiscard]] inline bool group_writable(unsigned mode) noexcept {
    return (mode & 020) != 0;
}

[[nodiscard]] inline int exit_for_query(bool writable) noexcept {
    return writable ? 0 : 1;
}

} // namespace pbsd::userland::usr_bin::mesg
