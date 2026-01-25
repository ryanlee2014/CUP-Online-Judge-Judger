#pragma once

#ifdef UNIT_TEST
#include <cstddef>
#include <cstdint>
#include <deque>
#include <string>
#include <vector>
#include <dlfcn.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/ptrace.h>
#include <unistd.h>
#include <seccomp.h>

namespace test_hooks {
struct ExitException {
    int code;
};

struct State {
    std::deque<int> fork_results;
    int default_fork_result = 123;
    int waitpid_status = 0;
    int wait4_status = 0;
    std::deque<int> wait4_statuses;
    long wait4_utime_ms = 0;
    long wait4_stime_ms = 0;
    int system_result = 0;
    std::deque<int> mysql_query_results;
    std::string mysql_error;
    std::string mysql_custom_input;
    std::string mysql_source;
    std::string mysql_user_id = "u";
    int mysql_problem_id = 1000;
    int mysql_language = 0;
    double mysql_time_limit = 1.0;
    int mysql_memory_limit = 128;
    int mysql_spj = 0;
    int compare_result = 4;
    int compile_result = 0;
    bool mysql_start_ok = true;
    std::string last_exec_path;
    std::string last_dlopen_path;
    int ptrace_syscall = 0;
    std::string pipe_seed;
    bool pipe_fail = false;
    int pipe_errno = 0;
    bool pipe_keep_open = false;
    int pipe_keep_fd = -1;
    bool exit_throws = true;
    int last_exit_code = 0;
};

State &state();
void reset();
void push_mysql_result(const std::vector<std::vector<std::string>> &rows);
} // namespace test_hooks

#ifdef __THROW
#define TEST_HOOKS_THROW __THROW
#else
#define TEST_HOOKS_THROW
#endif

#ifdef __cplusplus
extern "C" {
#endif
pid_t test_fork(void) TEST_HOOKS_THROW;
pid_t test_waitpid(pid_t pid, int *status, int options) TEST_HOOKS_THROW;
pid_t test_wait4(pid_t pid, int *status, int options, struct rusage *rusage) TEST_HOOKS_THROW;
int test_execvp(const char *file, char *const argv[]) TEST_HOOKS_THROW;
int test_execv(const char *path, char *const argv[]) TEST_HOOKS_THROW;
int test_execl(const char *path, const char *arg, ...) TEST_HOOKS_THROW;
long test_ptrace(enum __ptrace_request request, ...) TEST_HOOKS_THROW;
int test_setrlimit(int resource, const struct rlimit *rlim) TEST_HOOKS_THROW;
unsigned int test_alarm(unsigned int seconds) TEST_HOOKS_THROW;
int test_chroot(const char *path) TEST_HOOKS_THROW;
int test_setgid(gid_t gid) TEST_HOOKS_THROW;
int test_setuid(uid_t uid) TEST_HOOKS_THROW;
int test_setresuid(uid_t ruid, uid_t euid, uid_t suid) TEST_HOOKS_THROW;
int test_system(const char *command) TEST_HOOKS_THROW;
void *test_dlopen(const char *filename, int flag) TEST_HOOKS_THROW;
void *test_dlsym(void *handle, const char *symbol) TEST_HOOKS_THROW;
char *test_dlerror(void) TEST_HOOKS_THROW;
void test_exit(int code);
int test_pipe(int pipefd[2]) TEST_HOOKS_THROW;
int test_seccomp_load(scmp_filter_ctx ctx) TEST_HOOKS_THROW;
#ifdef __cplusplus
}
#endif

#define fork test_fork
#define waitpid test_waitpid
#define wait4 test_wait4
#define execvp test_execvp
#define execv test_execv
#define execl test_execl
#define ptrace test_ptrace
#define setrlimit test_setrlimit
#define alarm test_alarm
#define chroot test_chroot
#define setgid test_setgid
#define setuid test_setuid
#define setresuid test_setresuid
#define system test_system
#define dlopen test_dlopen
#define dlsym test_dlsym
#define dlerror test_dlerror
#define exit test_exit
#define _exit test_exit
#define pipe test_pipe
#define seccomp_load test_seccomp_load
#endif
