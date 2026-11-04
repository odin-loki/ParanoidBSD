module;
export module pbsd.userland.libc.msun.sinf;
import pbsd.userland.msun.sinf;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float sinf(float x) noexcept {
    return pbsd::userland::msun::sinf_val(x);
}
}