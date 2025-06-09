#include <algorithm>
#include <cstdio>
#include <iostream>
#include <signal.h>
#include <sstream>
#include <unistd.h>
#include <vector>

#include <libsdb/error.hpp>
#include <libsdb/process.hpp>
#include <readline/history.h>
#include <readline/readline.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

namespace {
std::unique_ptr<sdb::Process> attach(int argc, const char** argv) {
    // passing PID
    if (argc == 3 && argv[1] == std::string("-p")) {
        pid_t pid = std::atoi(argv[2]);
        return sdb::Process::attach(pid);
    }
    // passing program name
    else if (argc == 2) {
        const char* program_path = argv[1];
        return sdb::Process::launch(program_path);
    } else {
        std::cerr << "Usage: sdb <pid>/<program name>" << std::endl;
        return nullptr;
    }
}

std::vector<std::string> split(std::string_view str, char delimiter) {
    std::vector<std::string> out;
    std::stringstream ss{std::string(str)};
    std::string item;

    while (std::getline(ss, item, delimiter)) {
        out.push_back(item);
    }

    return out;
}

// str is a prefix of `of`?
bool is_prefix(std::string_view str, std::string_view of) {
    if (str.size() > of.size())
        return false;
    return std::equal(str.begin(), str.end(), of.begin());
}

void print_stop_reason(const sdb::Process& proc, const sdb::stop_reason& reason) {
    std::cout << "Process " << proc.pid() << " ";
    switch (reason.reason) {
        case sdb::process_state::stopped:
            std::cout << "stopped by signal " << strsignal(reason.info) << std::endl;
            break;
        case sdb::process_state::exited:
            std::cout << "exited with status " << static_cast<int>(reason.info) << std::endl;
            break;
        case sdb::process_state::terminated:
            std::cout << "terminated by signal " << strsignal(reason.info) << std::endl;
            break;
    }
}

void handle_command(std::unique_ptr<sdb::Process>& proc, std::string_view line) {
    auto args = split(line, ' ');
    auto cmd = args[0];

    if (is_prefix(cmd, "continue")) {
        proc->resume();
        auto reason = proc->wait_on_signal();
        print_stop_reason(*proc, reason);
    } else {
        std::cerr << "unkonw command" << std::endl;
    }
}
}  // namespace

void main_loop(std::unique_ptr<sdb::Process>& process) {
    char* line = nullptr;
    while ((line = readline("sdb> ")) != nullptr) {
        std::string line_str;

        if (line == std::string_view("")) {
            // empty line, use latest history
            free(line);
            if (history_length > 0) {
                line_str = history_list()[history_length - 1]->line;
            }
        } else {
            line_str = line;
            add_history(line);
            free(line);
        }

        if (!line_str.empty()) {
            try {
                handle_command(process, line_str);
            } catch (const sdb::Error& err) {
                std::cout << err.what() << std::endl;
            }
        }
    }
}

int main(int argc, const char** argv) {
    if (argc == 1) {
        std::cerr << "Usage: sdb <pid>/<program name>" << std::endl;
        return -1;
    }

    try {
        auto process = attach(argc, argv);
        main_loop(process);
    } catch (const sdb::Error& err) {
        std::cout << err.what() << std::endl;
    }
    return 0;
}