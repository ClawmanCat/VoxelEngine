#pragma once


// Getters
#define VE_GET_CREF(name)                                                       \
[[nodiscard]] const auto& get_##name(void) const { return name; }

#define VE_GET_MREF(name)                                                       \
VE_GET_CREF(name)                                                               \
[[nodiscard]] auto& get_##name(void) { return name; }

#define VE_GET_VAL(name)                                                        \
[[nodiscard]] auto get_##name(void) const { return name; }


#define VE_GET_STATIC_CREF(name)                                                \
[[nodiscard]] static const auto& get_##name(void) { return name; }

#define VE_GET_STATIC_MREF(name)                                                \
[[nodiscard]] static auto& get_##name(void) { return name; }

#define VE_GET_STATIC_VAL(name)                                                 \
[[nodiscard]] static auto get_##name(void) { return name; }


// Setters
#define VE_SET(name)                                                            \
void set_##name (const decltype(name)& value) { name = value; }                 \
void set_##name (decltype(name)&& value) { name = std::move(value); }

#define VE_STATIC_SET(name)                                                     \
static void set_##name (auto&& value) {                                         \
    name = std::forward<decltype(value)>(value);                                \
}


// Combinations
#define VE_GET_SET_CREF(name) VE_GET_CREF(name) VE_SET(name)
#define VE_GET_SET_MREF(name) VE_GET_MREF(name) VE_SET(name)
#define VE_GET_SET_VAL(name)  VE_GET_VAL(name)  VE_SET(name)

#define VE_GET_SET_STATIC_CREF(name) VE_GET_STATIC_CREF(name) VE_STATIC_SET(name)
#define VE_GET_SET_STATIC_MREF(name) VE_GET_STATIC_MREF(name) VE_STATIC_SET(name)
#define VE_GET_SET_STATIC_VAL(name)  VE_GET_STATIC_VAL(name)  VE_STATIC_SET(name)