#ifndef SDB_PROCESS_HPP
#define SDB_PROCESS_HPP

#include <filesystem>
#include <memory>

#include <libsdb/register.hpp>
#include <sys/types.h>

namespace sdb {

enum class process_state { stopped, running, exited, terminated };

struct stop_reason {
    stop_reason(int wait_status);
    process_state reason;  // reason for stop
    std::uint8_t info;     // info of stop
};

class Process {
   public:
    Process() = delete;
    Process(const Process&) = delete;
    Process& operator=(const Process&) = delete;

    static std::unique_ptr<Process> launch(const std::filesystem::path& path, bool debug = true);
    static std::unique_ptr<Process> attach(pid_t pid);

    void resume();
    pid_t pid() const { return pid_; }
    stop_reason wait_on_signal();

    ~Process();

    Register& get_register() { return *register_; }
    const Register& get_register() const { return *register_; }
    void write_user_area(std::size_t offset, std::uint64_t value);

   private:
    Process(pid_t pid, bool terminate_on_end, bool is_attached)
        : pid_(pid),
          terminate_on_end_(terminate_on_end),
          is_attached_(is_attached),
          register_(new Register(*this)) {}
    pid_t pid_ = 0;
    bool terminate_on_end_ = true;
    bool is_attached_ = true;
    process_state state_ = process_state::stopped;

    std::unique_ptr<Register> register_;
    void read_all_registers();  // read all register when process stopped
};

}  // namespace sdb

#endif  // SDB_PROCESS_HPP