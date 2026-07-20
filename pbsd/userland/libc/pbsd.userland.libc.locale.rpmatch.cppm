module;

export module pbsd.userland.libc.locale.rpmatch;

export import pbsd.core;

/// rpmatch from hbsd/src/lib/libc/locale/rpmatch.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Result<int> rpmatch_response(const char* resp) noexcept {
    if (resp == nullptr) {
        return result_err<int>(Status::Invalid);
    }
    switch (resp[0]) {
    case 'y':
    case 'Y':
        return result_ok(1);
    case 'n':
    case 'N':
        return result_ok(0);
    default:
        return result_err<int>(Status::Invalid);
    }
}

} // namespace pbsd::userland::libc
