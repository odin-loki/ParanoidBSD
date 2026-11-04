export module pbsd.kde.frameworks.kwidgets.messagewidget;

import pbsd.core;

/// Wave 3 pass 5 — KMessageWidget message type and layout constants.
/// Upstream: kde/frameworks/kwidgetsaddons/src/kmessagewidget.cpp
export namespace pbsd::kde::frameworks::kwidgets::messagewidget {

    enum class MessageType : unsigned char { Positive, Information, Warning, Error };
    inline constexpr unsigned kDefaultTimeoutMs = 5000;
    inline constexpr int kDefaultHeight = 32;
    inline constexpr int kIconSize = 22;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kwidgetsaddons/src/kmessagewidget.cpp";
}

} // namespace pbsd::kde::frameworks::kwidgets::messagewidget
