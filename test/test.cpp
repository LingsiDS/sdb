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
}  // namespace

TEST_CASE("process::launch success", "[process]") {
    auto proc = sdb::Process::launch("yes");
    REQUIRE(process_exists(proc->pid()));
}

TEST_CASE("process::launch no such program", "[process]") {
    // 测试启动不存在的程序，预期返回一个std::Error类型的exception
    REQUIRE_THROWS_AS(sdb::Process::launch("you_do_not_have_to_be_good"), sdb::Error);
}