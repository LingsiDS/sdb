#include <iostream>
#include <unistd.h>

#include <libsdb/error.hpp>
#include <libsdb/pipe.hpp>
#include <libsdb/process.hpp>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

namespace sdb {

void exit_with_perror(Pipe& channel, std::string const& prefix) {
    auto message = prefix + ": " + std::strerror(errno);
    channel.write(reinterpret_cast<std::byte*>(message.data()), message.size());
    exit(-1);
}

stop_reason::stop_reason(int wait_status) : reason(process_state::stopped), info(0) {
    if (WIFEXITED(wait_status)) {
        reason = process_state::exited;   // exited status
        info = WEXITSTATUS(wait_status);  // exit status
    } else if (WIFSIGNALED(wait_status)) {
        reason = process_state::terminated;  // terminated by signal
        info = WTERMSIG(wait_status);        // signal number
    } else if (WIFSTOPPED(wait_status)) {
        reason = process_state::stopped;  // stopped by signal
        info = WSTOPSIG(wait_status);     // signal number
    }
}

/**
 * @param debug:process only attach to target process when debug is true
 */
std::unique_ptr<Process> Process::launch(const std::filesystem::path& path, bool debug) {
    Pipe channel(/*close_on_exec=*/true);
    pid_t pid;
    if ((pid = fork()) < 0) {
        Error::send_errno("fork failed");
    }

    if (pid == 0) {
        channel.close_read();

        if (debug and ptrace(PTRACE_TRACEME, 0, nullptr, nullptr) < 0) {
            exit_with_perror(channel, "Tracing failed");
        }

        if (execlp(path.c_str(), path.c_str(), (char*)nullptr) < 0) {
            exit_with_perror(channel, "exec failed");  // write data through pipe to parent process
        }                                              // child process end here
    }

    channel.close_write();
    auto data = channel.read();
    channel.close_read();

    if (data.size() > 0) {
        waitpid(pid, nullptr, 0);
        // std::cout << "Parent: Received error from child" << std::flush << std::endl;
        std::string message(reinterpret_cast<char*>(data.data()), data.size());
        Error::send(message);
    }

    std::unique_ptr<Process> proc(new Process(pid, true, debug));
    if (debug)
        proc->wait_on_signal();

    return proc;
}

std::unique_ptr<Process> Process::attach(pid_t pid) {
    if (pid == 0) {
        Error::send("INVALID PID");
    }
    if (ptrace(PTRACE_ATTACH, pid, nullptr, nullptr) < 0) {
        Error::send_errno("ptrace failed, could not attach to process");
    }

    std::unique_ptr<Process> proc(new Process(pid, /*terminate_on_end=*/false, /*attached=*/true));
    proc->wait_on_signal();

    return proc;
}

void Process::resume() {
    if (ptrace(PTRACE_CONT, pid_, nullptr, nullptr) < 0) {
        Error::send_errno("ptrace failed, could not resume process");
    }
    state_ = process_state::running;
}

stop_reason Process::wait_on_signal() {
    int wait_status;
    int options = 0;
    // std::cout << "wait_on_signal: About to waitpid for pid=" << pid_ << std::flush << std::endl;

    if (waitpid(pid_, &wait_status, options) < 0) {
        // std::cout << "wait_on_signal: waitpid failed: " << strerror(errno) << std::flush
        //           << std::endl;
        Error::send_errno("waitpid failed");
    }

    // std::cout << "wait_on_signal: waitpid succeeded, status=" << wait_status << std::flush
    //           << std::endl;
    stop_reason reason(wait_status);
    state_ = reason.reason;
    return reason;
}

Process::~Process() {
    if (pid_ != 0) {
        int status;
        if (is_attached_) {
            if (state_ == process_state::running) {
                kill(pid_, SIGSTOP);  // process `pid_` whill stopped
                waitpid(pid_, &status, 0);
            }
            ptrace(PTRACE_DETACH, pid_, status, nullptr);
            kill(pid_, SIGCONT);  // process `pid_t` whill run continue

            if (terminate_on_end_) {
                kill(pid_, SIGKILL);  // kill the process `pid_'
                waitpid(pid_, &status, 0);
            }
        }
    }
}
}  // namespace sdb