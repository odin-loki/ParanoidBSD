module;

export module pbsd.userland.locale;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/locale/locale.c
export namespace pbsd::userland::usr_bin::locale {

enum class LocaleAction { List, Query, Set };

[[nodiscard]] inline LocaleAction locale_parse_flag(char c) noexcept {
    switch (c) {
    case 'a': return LocaleAction::List;
    case 'm': return LocaleAction::Query;
    default: return LocaleAction::Set;
    }
}

} // namespace pbsd::userland::usr_bin::locale
