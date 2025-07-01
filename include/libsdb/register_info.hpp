#ifndef SDB_REGISTER_INFO_HPP
#define SDB_REGISTER_INFO_HPP

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <error.h>
#include <string_view>

#include <sys/user.h>

namespace sdb {

enum class register_id {
// clang-format off
#define DEFINE_REGISTER(name, dwarf_id, size, offset, type, format) name
#include "detail/registers.inc"
#undef DEFINE_REGISTER
    // clang-format on
};

enum class register_type {
    gpr,      // general purpose register, e.g rax
    sub_gpr,  // sub register of gpr, e.g eax
    fpr,      // floating point register, e.g xmm0
    dr,       // debug register, e.g dr0
};

enum class register_format {
    uint,
    double_float,
    long_double,
    vector,
};

struct register_info {
    register_id id;
    std::string_view name;
    std::int32_t dwarf_id;
    std::size_t size;
    std::size_t offset;
    register_type type;
    register_format format;
};

inline constexpr register_info g_register_info[] = {
// clang-format off
    // Define the macro before including registers.inc
    #define DEFINE_REGISTER(name, dwarf_id, size, offset, type, format) \
        { register_id::name, #name, dwarf_id, size, offset, type, format }
    #include "detail/registers.inc"
    #undef DEFINE_REGISTER
    // clang-format on
};

template <class F>
const register_info& register_info_by(F f) {
    auto it = std::find_if(std::begin(g_register_info), std::end(g_register_info), f);
    if (it == std::end(g_register_info)) {
        Error::send("register not found");
    }
    return *it;
}

inline const register_info& register_info_by_id(register_id id) {
    return register_info_by([id](const auto& info) { return info.id == id; });
}

inline const register_info& register_info_by_name(std::string_view name) {
    return register_info_by([name](const auto& info) { return info.name == name; });
}

inline const register_info& register_info_by_dwarf_id(std::int32_t dwarf_id) {
    return register_info_by([dwarf_id](const auto& info) { return info.dwarf_id == dwarf_id; });
}

}  // namespace sdb
#endif  // SDB_REGISTER_INFO_HPP