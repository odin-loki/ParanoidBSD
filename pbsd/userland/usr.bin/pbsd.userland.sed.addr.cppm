module;
#include <cstddef>

export module pbsd.userland.sed.addr;

export import pbsd.core;
export import pbsd.userland.hosted;

export namespace pbsd::userland::usr_bin::sed::addr {

enum class AddrKind : unsigned char { Line, Regex, Last, Unknown };

struct Address {
    AddrKind kind{AddrKind::Unknown};
    int line{0};
};

[[nodiscard]] inline Result<Address> parse_address(const char* spec) noexcept {
    Address a{};
    if (spec == nullptr || spec[0] == '\0') {
        return result_err<Address>(Status::Invalid);
    }
    if (spec[0] == '$') {
        a.kind = AddrKind::Last;
        return result_ok(a);
    }
    int n = 0;
    for (const char* p = spec; *p >= '0' && *p <= '9'; ++p) {
        n = n * 10 + (*p - '0');
    }
    if (n > 0) {
        a.kind = AddrKind::Line;
        a.line = n;
        return result_ok(a);
    }
    a.kind = AddrKind::Regex;
    return result_ok(a);
}

}
