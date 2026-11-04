module;

export module pbsd.userland.syslogd;

export import pbsd.core;
export import pbsd.userland.hosted;

export namespace pbsd::userland::usr_sbin::syslogd {

[[nodiscard]] inline int priority_from_name(const char* name) noexcept {
    if (name == nullptr) {
        return -1;
    }
    if (hosted::cstrcmp(name, "info") == 0) {
        return 6;
    }
    if (hosted::cstrcmp(name, "debug") == 0) {
        return 7;
    }
    return -1;
}

}
