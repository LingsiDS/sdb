#ifndef SDB_REGISTER_HPP
#define SDB_REGISTER_HPP

#include <variant>

#include <libsdb/register_info.hpp>
#include <libsdb/types.hpp>
#include <sys/user.h>

namespace sdb {

class Process;
class Register {
   public:
    Register(register_id id) = delete;
    Register(const Register&) = delete;
    Register& operator=(const Register&) = delete;

    using value = std::variant<std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t,
                               std::int8_t, std::int16_t, std::int32_t, std::int64_t, float, double,
                               long double, byte64, byte128>;
    value read(const register_info& info) const;
    void write(const register_info& info, value value);

    template <class T>
    T read_by_id_as(register_id id) const {
        return std::get<T>(read(register_info_by_id(id)));
    }
    void write_by_id(register_id id, value value) { write(register_info_by_id(id), value); }

   private:
    friend class Process;
    Register(Process& proc) : proc_(&proc) {}  // only process can create a register

    user data_;  // all registers in here
    Process* proc_;
};

}  // namespace sdb

#endif  // SDB_REGISTER_HPP