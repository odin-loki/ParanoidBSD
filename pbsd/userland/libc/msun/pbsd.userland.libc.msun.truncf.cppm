module;
export module pbsd.userland.libc.msun.truncf;
import pbsd.userland.msun.truncf;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float truncf(float x) noexcept {
    return pbsd::userland::msun::truncf(x);
}
}
