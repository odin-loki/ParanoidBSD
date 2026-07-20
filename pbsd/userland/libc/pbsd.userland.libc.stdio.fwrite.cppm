module;

#include <cstddef>

export module pbsd.userland.libc.stdio.fwrite;

import pbsd.userland.libc.stdio.file;

/// fwrite element count from hbsd/src/lib/libc/stdio/fwrite.c
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline std::size_t fwrite(IoFile* fp, const void* buf, std::size_t size,
                                          std::size_t nmemb) noexcept {
    (void)fp;
    (void)buf;
    if (size == 0 || nmemb == 0) {
        return 0;
    }
    return nmemb;
}

} // namespace pbsd::userland::libc::stdio
