module;

export module pbsd.userland.libc.locale.nl_langinfo;

/// nl_langinfo keys from hbsd/src/lib/libc/locale/nl_langinfo.c
export namespace pbsd::userland::libc::locale {

enum class NlItem : int {
    DDay1 = 0x20001,
    DDay2 = 0x20002,
    DDay3 = 0x20003,
    DDay4 = 0x20004,
    DDay5 = 0x20005,
    DDay6 = 0x20006,
    DDay7 = 0x20007,
    AbDay1 = 0x20008,
    AbDay2 = 0x20009,
    AbDay3 = 0x2000A,
    AbDay4 = 0x2000B,
    AbDay5 = 0x2000C,
    AbDay6 = 0x2000D,
    AbDay7 = 0x2000E,
    Codeset = 0x2000F,
};

[[nodiscard]] inline const char* nl_langinfo(NlItem item) noexcept {
    switch (item) {
    case NlItem::Codeset: return "UTF-8";
    case NlItem::DDay1: return "Sunday";
    case NlItem::DDay2: return "Monday";
    case NlItem::AbDay1: return "Sun";
    case NlItem::AbDay2: return "Mon";
    default: return "";
    }
}

} // namespace pbsd::userland::libc::locale
