module;
export module pbsd.userland.libc.msun.scalbnf;
import pbsd.userland.msun.scalbnf;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float scalbnf(float x, int n) noexcept {
    return pbsd::userland::msun::scalbnf_val(x, n);
}
}
