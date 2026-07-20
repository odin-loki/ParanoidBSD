module;
export module pbsd.userland.libc.msun.logf;
import pbsd.userland.msun.logf;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float logf(float x) noexcept {
    return pbsd::userland::msun::logff_val(x);
}
}