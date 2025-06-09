#ifndef SDB_ERROR_HPP
#define SDB_ERROR_HPP

#include <cstring>
#include <stdexcept>
#include <string>

namespace sdb {

class Error : public std::runtime_error {
   public:
    [[noreturn]] static void send(const std::string& what) { throw Error(what); }
    [[noreturn]] static void send_errno(const std::string& prefix) {
        throw Error(prefix + ": " + std::strerror(errno));
    }

   private:
    Error(const std::string& what) : std::runtime_error(what) {}
    char error_code_[10];
};

}  // namespace sdb

#endif  // SDB_ERROR_HPP