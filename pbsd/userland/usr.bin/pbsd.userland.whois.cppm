module;
#include <cstddef>

export module pbsd.userland.whois;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/whois/whois.c — registry host selection (logic-only).
export namespace pbsd::userland::usr_bin::whois {

inline constexpr const char* kAbuseHost = "whois.abuse.net";
inline constexpr const char* kArinHost = "whois.arin.net";
inline constexpr const char* kDenicHost = "whois.denic.de";
inline constexpr const char* kDknicHost = "whois.dk-hostmaster.dk";
inline constexpr const char* kAfrinicHost = "whois.afrinic.net";
inline constexpr const char* kGnicHost = "whois.nic.gov";
inline constexpr const char* kIanahost = "whois.iana.org";
inline constexpr const char* kInternicHost = "whois.internic.net";
inline constexpr const char* kKnicHost = "whois.krnic.net";
inline constexpr const char* kLacnicHost = "whois.lacnic.net";
inline constexpr const char* kApnicHost = "whois.apnic.net";
inline constexpr const char* kRipeHost = "whois.ripe.net";
inline constexpr const char* kVerisignHost = "whois.verisign-grs.com";
inline constexpr const char* kDefaultPort = "whois";

inline constexpr unsigned kFlagRecurse = 0x01;
inline constexpr unsigned kFlagQuick = 0x02;
inline constexpr unsigned kFlagSpamMe = 0x04;

struct Options {
    unsigned flags{0};
    const char* host{nullptr};
};

[[nodiscard]] inline bool is_host_char(char c) noexcept {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '.' || c == '-';
}

[[nodiscard]] inline bool valid_query(const char* query) noexcept {
    if (query == nullptr || query[0] == '\0') {
        return false;
    }
    for (const char* p = query; *p != '\0'; ++p) {
        if (!is_host_char(*p)) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] inline bool is_ip_like(const char* query) noexcept {
    if (query == nullptr) {
        return false;
    }
    int dots = 0;
    for (const char* p = query; *p != '\0'; ++p) {
        if (*p == '.') {
            ++dots;
        } else if (*p < '0' || *p > '9') {
            return false;
        }
    }
    return dots >= 1;
}

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        const char* arg = argv[i];
        for (int j = 1; arg[j] != '\0'; ++j) {
            switch (arg[j]) {
            case 'h':
                opt.host = arg + 2;
                if (arg[2] == '\0') {
                    if (i + 1 >= argc) {
                        return result_err<Options>(Status::Invalid);
                    }
                    opt.host = argv[++i];
                }
                break;
            case 'Q':
            case 'q':
                opt.flags |= kFlagQuick;
                break;
            case 'a':
                opt.flags |= kFlagRecurse;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    if (argc - i < 1) {
        return result_err<Options>(Status::Invalid);
    }
    if (!valid_query(argv[i])) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::whois
