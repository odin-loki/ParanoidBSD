module;
export module pbsd.userland.libc.msun.log10f;
import pbsd.userland.msun.log10f;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float log10f(float x) noexcept {
    return pbsd::userland::msun::log10f_val(x);
}
}