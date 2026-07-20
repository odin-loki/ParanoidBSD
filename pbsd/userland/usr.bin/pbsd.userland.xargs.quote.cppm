module;

export module pbsd.userland.xargs.quote;

export import pbsd.core;

export namespace pbsd::userland::usr_bin::xargs::quote {

[[nodiscard]] inline bool needs_quoting(const char* arg) noexcept {
    if (arg == nullptr) {
        return false;
    }
    for (const char* p = arg; *p; ++p) {
        if (*p == ' ' || *p == '\t' || *p == '\'' || *p == '"') {
            return true;
        }
    }
    return false;
}

}
