module;

export module pbsd.userland.libc.locale.localeconv;

/// localeconv stub from hbsd/src/lib/libc/locale/localeconv.c
export namespace pbsd::userland::libc::locale {

struct LConv {
    const char* decimal_point = ".";
    const char* thousands_sep = "";
    const char* grouping = "";
    const char* mon_decimal_point = "";
    const char* mon_thousands_sep = "";
    const char* mon_grouping = "";
    const char* positive_sign = "";
    const char* negative_sign = "-";
    const char* currency_symbol = "";
    char frac_digits{'\0'};
    char p_cs_precedes{'\0'};
    char n_cs_precedes{'\0'};
    char p_sep_by_space{'\0'};
    char n_sep_by_space{'\0'};
    char p_sign_posn{'\0'};
    char n_sign_posn{'\0'};
    char int_frac_digits{'\0'};
    char int_p_cs_precedes{'\0'};
    char int_n_cs_precedes{'\0'};
    char int_p_sep_by_space{'\0'};
    char int_n_sep_by_space{'\0'};
    char int_p_sign_posn{'\0'};
    char int_n_sign_posn{'\0'};
};

[[nodiscard]] inline const LConv& localeconv() noexcept {
    static const LConv c{};
    return c;
}

} // namespace pbsd::userland::libc::locale
