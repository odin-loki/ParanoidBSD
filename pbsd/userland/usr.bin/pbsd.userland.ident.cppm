module;

export module pbsd.userland.ident;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/ident/ident.c
export namespace pbsd::userland::usr_bin::ident {

[[nodiscard]] inline bool ident_match_keyword(const char* kw, char c) noexcept {
    return kw != nullptr && kw[0] == c;
}

} // namespace pbsd::userland::usr_bin::ident
