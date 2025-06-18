#include <fcntl.h>
#include <unistd.h>
#include <utility>

#include <libsdb/error.hpp>
#include <libsdb/pipe.hpp>

namespace sdb {
Pipe::Pipe(bool close_on_exec) {
    if (pipe2(fds_, close_on_exec ? O_CLOEXEC : 0) < 0) {
        Error::send_errno("Pipe creation failed");
    }
}

Pipe::~Pipe() {
    close_read();
    close_write();
}

int Pipe::release_write() {
    return std::exchange(fds_[write_fd], -1);
}

int Pipe::release_read() {
    return std::exchange(fds_[read_fd], -1);
}

void Pipe::close_read() {
    if (fds_[read_fd] != -1) {
        close(fds_[read_fd]);
        fds_[read_fd] = -1;
    }
}

void Pipe::close_write() {
    if (fds_[write_fd] != -1) {
        close(fds_[write_fd]);
        fds_[write_fd] = -1;
    }
}

std::vector<std::byte> Pipe::read() {
    char buf[1024];
    auto n = ::read(fds_[read_fd], buf, sizeof(buf));
    if (n < 0) {
        Error::send_errno("Pipe read failed");
    }
    return std::vector<std::byte>(reinterpret_cast<std::byte*>(buf),
                                  reinterpret_cast<std::byte*>(buf + n));
}

void Pipe::write(std::byte* from, std::size_t bytes) {
    if (::write(fds_[write_fd], from, bytes) < 0) {
        Error::send_errno("Pipe write failed");
    }
}
}  // namespace sdb