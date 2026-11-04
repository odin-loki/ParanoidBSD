module;

export module pbsd.userland.libc.stdio.fclose;

import pbsd.core;
import pbsd.userland.libc.stdio.file;

/// fclose from hbsd/src/lib/libc/stdio/fclose.c
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline Status fclose(IoFile& fp) noexcept {
    fp.fd = -1;
    clear_flags(fp);
    return Status::Ok;
}

} // namespace pbsd::userland::libc::stdio
