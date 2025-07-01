#include <iostream>

#include <libsdb/bit.hpp>
#include <libsdb/error.hpp>
#include <libsdb/process.hpp>
#include <libsdb/register.hpp>

namespace sdb {

Register::value Register::read(const register_info& info) const {
    auto bytes = as_bytes(data_);

    if (info.format == register_format::uint) {
        switch (info.size) {
            case 1:
                return from_bytes<std::uint8_t>(bytes + info.offset);
            case 2:
                return from_bytes<std::uint16_t>(bytes + info.offset);
            case 4:
                return from_bytes<std::uint32_t>(bytes + info.offset);
            case 8:
                return from_bytes<std::uint64_t>(bytes + info.offset);
            default:
                Error::send("Unexpected register size");
        }
    } else if (info.format == register_format::double_float) {
        return from_bytes<double>(bytes + info.offset);
    } else if (info.format == register_format::long_double) {
        return from_bytes<long double>(bytes + info.offset);
    } else if (info.format == register_format::vector && info.size == 8) {
        return from_bytes<byte64>(bytes + info.offset);
    } else if (info.format == register_format::vector && info.size == 16) {
        return from_bytes<byte128>(bytes + info.offset);
    } else {
        Error::send("Unexpected register format");
    }
}

void Register::write(const register_info& info, value val) {
    auto bytes = as_bytes(data_);

    std::visit(
        [&](auto& v) {  // callable, can process all type in std::variant
            if (sizeof(v) == info.size) {
                auto val_bytes = as_bytes(v);
                std::copy(val_bytes, val_bytes + sizeof(v), bytes + info.offset);
            } else {
                std::cerr << "sdb::Register::write call with mismatch register and value sizes";
                std::terminate();
            }
        },
        val);

    // modify process register
    proc_->write_user_area(info.offset, from_bytes<std::uint64_t>(bytes + info.offset));
}

}  // namespace sdb