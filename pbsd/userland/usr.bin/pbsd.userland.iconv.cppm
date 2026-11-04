module;

export module pbsd.userland.iconv;

export import pbsd.core;
export import pbsd.userland.hosted;

export namespace pbsd::userland::usr_bin::iconv {

[[nodiscard]] inline bool charset_is_utf8(const char* cs) noexcept {
    return cs != nullptr
        && (hosted::cstrcmp(cs, "UTF-8") == 0 || hosted::cstrcmp(cs, "utf-8") == 0);
}

}
