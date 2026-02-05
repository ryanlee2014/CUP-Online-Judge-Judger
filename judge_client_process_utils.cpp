#include "judge_client_process_utils.h"

#include <cstdlib>
#include <unistd.h>

pid_t spawn_child(const std::function<void()> &fn) {
    pid_t pid = fork();
    if (pid == 0) {
        fn();
        exit(0);
    }
    return pid;
}
