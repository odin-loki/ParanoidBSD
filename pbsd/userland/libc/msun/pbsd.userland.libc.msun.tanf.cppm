module;
export module pbsd.userland.libc.msun.tanf;
import pbsd.userland.msun.tanf;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float tanf(float x) noexcept {
    return pbsd::userland::msun::tanf_val(x);
}
}