#include <fstream>
#include <signal.h>

#include <catch2/catch_test_macros.hpp>
#include <libsdb/error.hpp>
#include <libsdb/process.hpp>
#include <sys/types.h>

namespace {
bool process_exists(pid_t pid) {
    auto ret = kill(pid, 0);
    return ret != -1 && errno != ESRCH;
}

char get_process_status(pid_t pid) {
    std::ifstream stat("/proc/" + std::to_string(pid) + "/stat");
    std::string data;

    std::getline(stat, data);
    auto index_of_last_parenthesis = data.rfind(')');
    auto index_of_status_indicator = index_of_last_parenthesis + 2;
    return data[index_of_status_indicator];
}

}  // namespace

TEST_CASE("process::launch success", "[process]") {
    auto proc = sdb::Process::launch("yes");
    REQUIRE(process_exists(proc->pid()));
}

TEST_CASE("process::launch no such program", "[process]") {
    // 测试启动不存在的程序，预期返回一个std::Error类型的exception
    REQUIRE_THROWS_AS(sdb::Process::launch("you_do_not_have_to_be_good"), sdb::Error);
}

TEST_CASE("process::attach success", "[process]") {
    /*test attach process*/
    auto target = sdb::Process::launch("build/test/targets/run_endlessly",
                                       false);  // launch a process, but don't attach it
    auto proc = sdb::Process::attach(target->pid());
    REQUIRE(get_process_status(target->pid()) ==
            't');  // when attach success, target process status in Tracing stop(t)
}

TEST_CASE("process::attach invalid PID", "[process]") {
    REQUIRE_THROWS_AS(sdb::Process::attach(0), sdb::Error);
}

TEST_CASE("process::resume success", "[process]") {
    {
        auto proc = sdb::Process::launch("build/test/targets/run_endlessly", true);
        proc->resume();
        auto status = get_process_status(proc->pid());
        auto success = status == 'R' or status == 'S';
        REQUIRE(success);
    }
    {
        auto target = sdb::Process::launch("build/test/targets/run_endlessly", false);
        auto proc = sdb::Process::attach(target->pid());
        proc->resume();
        auto status = get_process_status(proc->pid());
        auto success = status == 'R' or status == 'S';
        REQUIRE(success);
    }
}

TEST_CASE("process::resume already terminated", "[process]") {
    auto proc = sdb::Process::launch("build/test/targets/end_immediately", true);
    proc->resume();
    proc->wait_on_signal();  // wait process terminate
    REQUIRE_THROWS_AS(proc->resume(), sdb::Error);
}