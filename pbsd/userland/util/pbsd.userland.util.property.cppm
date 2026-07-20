module;

#include <cctype>
#include <cstdlib>
#include <cstring>

export module pbsd.userland.util.property;

import pbsd.core;

/// properties_read(3) state machine from hbsd/src/lib/libutil/property.c
export namespace pbsd::userland::util {

inline constexpr unsigned PROPERTY_MAX_NAME  = 64;
inline constexpr unsigned PROPERTY_MAX_VALUE = 512;

struct PropertyEntry {
    PropertyEntry* next{nullptr};
    char* name{nullptr};
    char* value{nullptr};
};

using PropertyList = PropertyEntry*;

namespace detail {

[[nodiscard]] inline PropertyEntry* property_alloc(char* name, char* value) noexcept {
    auto* n = static_cast<PropertyEntry*>(std::malloc(sizeof(PropertyEntry)));
    if (n == nullptr) {
        return nullptr;
    }
    n->next = nullptr;
    if (name != nullptr) {
        n->name = static_cast<char*>(std::malloc(std::strlen(name) + 1));
        if (n->name == nullptr) {
            std::free(n);
            return nullptr;
        }
        std::strcpy(n->name, name);
    } else {
        n->name = nullptr;
    }
    if (value != nullptr) {
        n->value = static_cast<char*>(std::malloc(std::strlen(value) + 1));
        if (n->value == nullptr) {
            std::free(n->name);
            std::free(n);
            return nullptr;
        }
        std::strcpy(n->value, value);
    } else {
        n->value = nullptr;
    }
    return n;
}

enum class ParseState {
    Look, Comment, Name, Value, MValue, Commit, Fill, Stop
};

} // namespace detail

inline void properties_free(PropertyList list) noexcept {
    while (list != nullptr) {
        PropertyList tmp = list->next;
        if (list->name != nullptr) {
            std::free(list->name);
        }
        if (list->value != nullptr) {
            std::free(list->value);
        }
        std::free(list);
        list = tmp;
    }
}

/// Read property list from fd using the libutil FSM (Wave 2: hosted read callback).
template<typename ReadFn>
[[nodiscard]] inline PropertyList properties_read(ReadFn read_fn) noexcept {
    PropertyList head = nullptr;
    PropertyList ptr = nullptr;
    char hold_n[PROPERTY_MAX_NAME + 1]{};
    char hold_v[PROPERTY_MAX_VALUE + 1]{};
    char buf[4096]{};
    int bp = 0, n = 0, v = 0, max = 0;
    detail::ParseState state = detail::ParseState::Look;
    detail::ParseState last_state = detail::ParseState::Look;
    int ch = 0;
    int blevel = 0;

    while (state != detail::ParseState::Stop) {
        if (state != detail::ParseState::Commit) {
            if (bp == max) {
                last_state = state;
                state = detail::ParseState::Fill;
            } else {
                ch = buf[bp++];
            }
        }
        switch (state) {
        case detail::ParseState::Fill: {
            max = read_fn(buf, sizeof(buf));
            if (max < 0) {
                properties_free(head);
                return nullptr;
            }
            if (max == 0) {
                state = detail::ParseState::Stop;
            } else {
                state = last_state;
                ch = buf[0];
                bp = 0;
            }
            continue;
        }
        case detail::ParseState::Look:
            if (std::isspace(static_cast<unsigned char>(ch))) {
                continue;
            }
            if (ch == '#' || ch == ';') {
                state = detail::ParseState::Comment;
                continue;
            }
            if (std::isalnum(static_cast<unsigned char>(ch)) || ch == '_') {
                if (n >= static_cast<int>(PROPERTY_MAX_NAME)) {
                    n = 0;
                    state = detail::ParseState::Comment;
                } else {
                    hold_n[n++] = static_cast<char>(ch);
                    state = detail::ParseState::Name;
                }
            } else {
                state = detail::ParseState::Comment;
            }
            break;
        case detail::ParseState::Comment:
            if (ch == '\n') {
                state = detail::ParseState::Look;
            }
            break;
        case detail::ParseState::Name:
            if (ch == '\n' || !ch) {
                hold_n[n] = '\0';
                hold_v[0] = '\0';
                v = n = 0;
                state = detail::ParseState::Commit;
            } else if (std::isspace(static_cast<unsigned char>(ch))) {
                continue;
            } else if (ch == '=') {
                hold_n[n] = '\0';
                v = n = 0;
                state = detail::ParseState::Value;
            } else {
                hold_n[n++] = static_cast<char>(ch);
            }
            break;
        case detail::ParseState::Value:
            if (v == 0 && ch == '\n') {
                hold_v[v] = '\0';
                v = n = 0;
                state = detail::ParseState::Commit;
            } else if (v == 0 && std::isspace(static_cast<unsigned char>(ch))) {
                continue;
            } else if (ch == '{') {
                state = detail::ParseState::MValue;
                ++blevel;
            } else if (ch == '\n' || !ch) {
                hold_v[v] = '\0';
                v = n = 0;
                state = detail::ParseState::Commit;
            } else {
                if (v >= static_cast<int>(PROPERTY_MAX_VALUE)) {
                    state = detail::ParseState::Comment;
                    v = n = 0;
                    break;
                }
                hold_v[v++] = static_cast<char>(ch);
            }
            break;
        case detail::ParseState::MValue:
            if (v >= static_cast<int>(PROPERTY_MAX_VALUE)) {
                state = detail::ParseState::Comment;
                n = v = 0;
            } else if (ch == '}' && !--blevel) {
                hold_v[v] = '\0';
                v = n = 0;
                state = detail::ParseState::Commit;
            } else {
                hold_v[v++] = static_cast<char>(ch);
                if (ch == '{') {
                    ++blevel;
                }
            }
            break;
        case detail::ParseState::Commit:
            if (head == nullptr) {
                head = ptr = detail::property_alloc(hold_n, hold_v);
                if (head == nullptr) {
                    return nullptr;
                }
            } else {
                ptr->next = detail::property_alloc(hold_n, hold_v);
                if (ptr->next == nullptr) {
                    properties_free(head);
                    return nullptr;
                }
                ptr = ptr->next;
            }
            state = detail::ParseState::Look;
            v = n = 0;
            break;
        case detail::ParseState::Stop:
            break;
        }
    }

    if (head == nullptr) {
        head = detail::property_alloc(nullptr, nullptr);
    }
    return head;
}

[[nodiscard]] inline char* property_find(PropertyList list,
                                         const char* name) noexcept {
    if (list == nullptr || name == nullptr || name[0] == '\0') {
        return nullptr;
    }
    while (list != nullptr) {
        if (list->name != nullptr && std::strcmp(list->name, name) == 0) {
            return list->value;
        }
        list = list->next;
    }
    return nullptr;
}

[[nodiscard]] inline unsigned property_count(PropertyList list) noexcept {
    unsigned count = 0;
    while (list != nullptr) {
        ++count;
        list = list->next;
    }
    return count;
}

} // namespace pbsd::userland::util
