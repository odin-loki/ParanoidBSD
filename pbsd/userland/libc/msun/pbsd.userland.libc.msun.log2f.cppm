module;
export module pbsd.userland.libc.msun.log2f;
import pbsd.userland.msun.log2f;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float log2f(float x) noexcept {
    return pbsd::userland::msun::log2f_val(x);
}
}
