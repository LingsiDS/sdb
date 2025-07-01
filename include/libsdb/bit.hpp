#ifndef SDB_BIT_HPP
#define SDB_BIT_HPP

#include <cstddef>
#include <cstring>

#include <libsdb/types.hpp>

namespace sdb {
template <class To>
To from_bytes(const std::byte* byte) {
    To value;
    std::memcpy(&value, byte, sizeof(To));
    return value;
}

template <class From>
std::byte* as_bytes(From& from) {
    return reinterpret_cast<std::byte*>(&from);
}

template <class From>
const std::byte* as_bytes(const From& from) {
    return reinterpret_cast<const std::byte*>(&from);
}

template <class From>
byte128 to_bytes128(From from) {
    byte128 bytes{};  // init all bytes to 0
    std::memcpy(&bytes, &from, sizeof(From));
    return bytes;
}

template <class From>
byte64 to_bytes64(From from) {
    byte64 bytes{};
    std::memcpy(&bytes, &from, sizeof(From));
    return bytes;
}
}  // namespace sdb

#endif  // SDB_BIT_HPP