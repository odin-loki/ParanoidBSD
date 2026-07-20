module;
export module pbsd.userland.libc.msun.atanf;
import pbsd.userland.msun.atanf;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float atanf(float x) noexcept {
    return pbsd::userland::msun::atanf_val(x);
}
}