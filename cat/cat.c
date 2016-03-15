#include <unistd.h>
#include "helpers.h"

#define BUF_SIZE 4000

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
