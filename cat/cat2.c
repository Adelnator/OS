#include <unistd.h>

#define BUF_SIZE 4000

ssize_t read_(int fd, void *buf, size_t count) {
    ssize_t cur = 0;
    while (cur < count) {
        ssize_t res = read(fd, buf + cur, count - cur);
        if (res == -1) return -1;
        if (res == 0) break;
        cur += res;
    }
    return cur;
}

ssize_t write_(int fd, const void *buf, size_t count) {
    ssize_t cur = 0;
    while (cur < count) {
        ssize_t written = write(fd, buf + cur, count - cur);
        if (written == -1) return -1;
        if (written == 0) break;
        cur += written;
    }
    return cur;
}

int main() {
    char buf[BUF_SIZE];
    while(1) {
        ssize_t res = read_(STDIN_FILENO, buf, BUF_SIZE);
        if (res <= 0) return res;
        ssize_t written = write_(STDOUT_FILENO, buf, res);
        if (written == -1) return written;
        if (written < BUF_SIZE) return 0;
    }
}
