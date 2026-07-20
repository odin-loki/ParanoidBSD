module;
#include <cstddef>

export module pbsd.userland.grep.context;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/grep/grep.c — Context line (-A/-B/-C) helpers
export namespace pbsd::userland::usr_bin::grep::context {

struct Context {
    int after{0};
    int before{0};
};

[[nodiscard]] inline Result<Context> parse_context_flag(const char* arg) noexcept {
    Context ctx{};
    if (arg == nullptr || arg[0] != '-') {
        return result_err<Context>(Status::Invalid);
    }
    char kind = arg[1];
    if (kind != 'A' && kind != 'B' && kind != 'C') {
        return result_err<Context>(Status::Invalid);
    }
    int n = 0;
    for (const char* p = arg + 2; *p >= '0' && *p <= '9'; ++p) {
        n = n * 10 + (*p - '0');
    }
    if (kind == 'A') {
        ctx.after = n;
    } else if (kind == 'B') {
        ctx.before = n;
    } else {
        ctx.after = ctx.before = n;
    }
    return result_ok(ctx);
}

} // namespace pbsd::userland::usr_bin::grep::context
