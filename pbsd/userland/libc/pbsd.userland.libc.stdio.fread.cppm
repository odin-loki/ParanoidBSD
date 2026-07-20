module;

#include <cstddef>

export module pbsd.userland.libc.stdio.fread;

import pbsd.userland.libc.stdio.file;

/// fread element count from hbsd/src/lib/libc/stdio/fread.c
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline std::size_t fread_elements(std::size_t size, std::size_t nmemb) noexcept {
    if (size == 0 || nmemb == 0) {
        return 0;
    }
    return nmemb;
}

[[nodiscard]] inline std::size_t fread_bytes(std::size_t size, std::size_t nmemb) noexcept {
    return size * fread_elements(size, nmemb);
}

[[nodiscard]] inline std::size_t fread(IoFile* fp, void* buf, std::size_t size,
                                         std::size_t nmemb) noexcept {
    (void)fp;
    (void)buf;
    return fread_elements(size, nmemb);
}

} // namespace pbsd::userland::libc::stdio
