module;
#include <cstdint>

export module pbsd.kernel.audit_arg;

import pbsd.core;

/// Freestanding port of `security/audit/audit_arg.c` — audit argument helpers.
export namespace pbsd::kernel::audit_arg {

enum class ArgKind : unsigned char {
    None = 0,
    Int = 1,
    String = 2,
    Opaque = 3,
};

struct Arg {
    ArgKind kind{ArgKind::None};
    std::uint64_t value{};
};

[[nodiscard]] inline Status validate(const Arg& arg) noexcept {
    switch (arg.kind) {
    case ArgKind::None:
    case ArgKind::Int:
    case ArgKind::String:
    case ArgKind::Opaque:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline Status encode_int(Arg& arg, std::uint64_t v) noexcept {
    arg.kind = ArgKind::Int;
    arg.value = v;
    return Status::Ok;
}

} // namespace pbsd::kernel::audit_arg
