#include "judge_client_watch_helpers.h"

#include <algorithm>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

#include <sys/ptrace.h>
#include <sys/resource.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

#include "header/static_var.h"
#include "library/judge_lib.h"
#include "model/judge/language/Language.h"

using namespace std;

namespace judge_watch_helpers {

std::string join_path(const char *base, const char *name) {
    return (std::filesystem::path(base) / name).string();
}

#ifdef __i386
#define REG_SYSCALL orig_eax
#define REG_RET eax
#define REG_ARG0 ebx
#define REG_ARG1 ecx
#else
#define REG_SYSCALL orig_rax
#define REG_RET rax
#define REG_ARG0 rdi
#define REG_ARG1 rsi
#endif

extern int call_counter[call_array_size];

void build_parallel_error_name(int file_id, char *errorOutput) {
    snprintf(errorOutput, BUFFER_SIZE, "error%d.out", file_id);
}

static bool handle_error_conditions(shared_ptr<Language> &languageModel, const char *errorFile, int &ACflg,
                                    int solution_id, pid_t pidApp, long &last_error_size,
                                    bool debug_enabled, const JudgeConfigSnapshot &config, const char *work_dir) {
    long error_size = get_file_size(errorFile);
    bool has_error = error_size > 0;
    if (has_error && error_size != last_error_size) {
        if (debug_enabled) {
            cerr << "Catch error:" << endl;
            fstream err(errorFile);
            stringstream ss;
            ss << err.rdbuf();
            cerr << ss.str() << endl;
        }
        fstream file(errorFile, ios::ate);
        stringstream buffer;
        buffer << file.rdbuf();
        string contents(buffer.str());
        if (contents.find("Killed") != std::string::npos) {
            write_log(oj_home, contents.c_str());
            print_runtimeerror(contents.c_str(), work_dir);
            last_error_size = error_size;
            return true;
        }
        last_error_size = error_size;
    }
    if (languageModel->gotErrorWhileRunning(has_error) && !config.all_test_mode) {
        ACflg = RUNTIME_ERROR;
        if (config.use_ptrace) {
            ptrace(PTRACE_KILL, pidApp, NULL, NULL);
        }
        return true;
    }
    return false;
}

static bool handle_output_limit(int isspj, const char *userfile, long outfile_size, int &ACflg, pid_t pidApp,
                                const JudgeConfigSnapshot &config) {
    if (!isspj && get_file_size(userfile) > outfile_size * 2 + 1024) {
        ACflg = OUTPUT_LIMIT_EXCEEDED;
        if (config.use_ptrace) {
            ptrace(PTRACE_KILL, pidApp, NULL, NULL);
        }
        return true;
    }
    return false;
}

static int map_signal_to_ac(int signal_code) {
    switch (signal_code) {
        case SIGCHLD:
        case SIGALRM:
            alarm(0);
        case SIGKILL:
        case SIGXCPU:
            return TIME_LIMIT_EXCEEDED;
        case SIGXFSZ:
            return OUTPUT_LIMIT_EXCEEDED;
        default:
            return RUNTIME_ERROR;
    }
}

static bool handle_exit_status(shared_ptr<Language> &languageModel, int status, int &ACflg, pid_t pidApp,
                               bool debug_enabled, const JudgeConfigSnapshot &config, const char *work_dir) {
    int exitcode = WEXITSTATUS(status);
    if (languageModel->isValidExitCode(exitcode)) {
        return false;
    }
    if (debug_enabled) {
        printf("status>>8=%d\n", exitcode);
    }
    if (ACflg == ACCEPT) {
        ACflg = map_signal_to_ac(exitcode);
        print_runtimeerror(strsignal(exitcode), work_dir);
    }
    if (config.use_ptrace) {
        ptrace(PTRACE_KILL, pidApp, NULL, NULL);
    }
    return true;
}

static bool handle_signal_status(int status, int &ACflg, bool debug_enabled, const char *work_dir) {
    if (!WIFSIGNALED(status)) {
        return false;
    }
    int sig = WTERMSIG(status);
    if (debug_enabled) {
        printf("WTERMSIG=%d\n", sig);
        psignal(sig, nullptr);
    }
    if (ACflg == ACCEPT) {
        ACflg = map_signal_to_ac(sig);
        print_runtimeerror(strsignal(sig), work_dir);
    }
    return true;
}

static void handle_ptrace_syscall(pid_t pidApp, int &ACflg, int solution_id, int *call_counter_local,
                                  bool record_syscall, const JudgeConfigSnapshot &config, const char *work_dir) {
    if (config.use_ptrace) {
        struct user_regs_struct reg{};
        ptrace(PTRACE_GETREGS, pidApp, NULL, &reg);
        if (call_counter_local[reg.REG_SYSCALL]) {
        } else if (record_syscall) {
            call_counter_local[reg.REG_SYSCALL] = 1;
        } else {
            ACflg = RUNTIME_ERROR;
            string _error;
            _error = string("Current Program use not allowed system call.\nSolution ID:") + to_string(solution_id) +
                     "\n";
            _error += string("Syscall ID:") + to_string(reg.REG_SYSCALL) + "\n";
            write_log(oj_home, _error.c_str());
            print_runtimeerror(_error.c_str(), work_dir);
            ptrace(PTRACE_KILL, pidApp, NULL, NULL);
        }
        ptrace(PTRACE_SYSCALL, pidApp, NULL, NULL);
    }
}

static bool update_memory_and_check(shared_ptr<Language> &languageModel, const struct rusage &ruse, pid_t pidApp,
                                    int mem_lmt, int &topmemory, int &ACflg,
                                    bool debug_enabled, const JudgeConfigSnapshot &config) {
    int tempmemory = languageModel->getMemory(ruse, pidApp);
    topmemory = max(tempmemory, topmemory);
    if (topmemory > mem_lmt * STD_MB) {
        if (debug_enabled)
            printf("out of memory %d\n", topmemory);
        if (ACflg == ACCEPT)
            ACflg = MEMORY_LIMIT_EXCEEDED;
        if (config.use_ptrace) {
            ptrace(PTRACE_KILL, pidApp, NULL, NULL);
        }
        return true;
    }
    return false;
}

static void add_usedtime(double &usedtime, const struct rusage &ruse) {
    usedtime += (ruse.ru_utime.tv_sec * 1000 + ruse.ru_utime.tv_usec / 1000);
    usedtime += (ruse.ru_stime.tv_sec * 1000 + ruse.ru_stime.tv_usec / 1000);
}

static bool wait_and_update(pid_t pidApp, int &status, struct rusage &ruse, shared_ptr<Language> &languageModel,
                            int mem_lmt, int &topmemory, int &ACflg, bool debug_enabled,
                            const JudgeConfigSnapshot &config) {
    wait4(pidApp, &status, 0, &ruse);
    return update_memory_and_check(languageModel, ruse, pidApp, mem_lmt, topmemory, ACflg, debug_enabled, config);
}

static bool process_watch_iteration(pid_t pidApp, int &status, struct rusage &ruse, shared_ptr<Language> &languageModel,
                                    int mem_lmt, int &topmemory, int &ACflg, int isspj, const char *userfile,
                                    const char *outfile, const char *errorFile, int solution_id, int *call_counter_local,
                                    long &last_error_size, long outfile_size, bool check_io, bool record_syscall,
                                    bool debug_enabled,
                                    const JudgeConfigSnapshot &config, const char *work_dir) {
    if (wait_and_update(pidApp, status, ruse, languageModel, mem_lmt, topmemory, ACflg, debug_enabled, config)) {
        return true;
    }
    if (WIFEXITED(status))
        return true;
    if (check_io) {
        if (handle_error_conditions(languageModel, errorFile, ACflg, solution_id, pidApp, last_error_size,
                                    debug_enabled, config, work_dir)) {
            return true;
        }
    }
    if (handle_output_limit(isspj, userfile, outfile_size, ACflg, pidApp, config)) {
        return true;
    }
    if (handle_exit_status(languageModel, status, ACflg, pidApp, debug_enabled, config, work_dir)) {
        return true;
    }
    if (handle_signal_status(status, ACflg, debug_enabled, work_dir)) {
        return true;
    }
    handle_ptrace_syscall(pidApp, ACflg, solution_id, call_counter_local, record_syscall, config, work_dir);
    return false;
}

static bool should_check_io(int &io_tick,
                            std::chrono::steady_clock::time_point &last_io_check,
                            int interval_ms,
                            int tick_interval) {
    ++io_tick;
    if (io_tick % tick_interval == 0) {
        last_io_check = std::chrono::steady_clock::now();
        return true;
    }
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_io_check).count();
    if (elapsed >= interval_ms) {
        last_io_check = now;
        return true;
    }
    return false;
}

void watch_solution_common(pid_t pidApp, char *infile, int &ACflg, int isspj,
                           char *userfile, char *outfile, int solution_id, int lang,
                           int &topmemory, int mem_lmt, double &usedtime,
                           const char *errorFile, int *call_counter_local,
                           const JudgeConfigSnapshot &config, const char *work_dir, bool record_syscall,
                           bool debug_enabled) {
    shared_ptr<Language> languageModel(getLanguageModel(lang));
    if (debug_enabled) {
        printf("pid=%d judging %s\n", pidApp, infile);
    }
    int status;
    struct rusage ruse{};
    long last_error_size = -1;
    if (topmemory == 0) {
        topmemory = get_proc_status(pidApp, "VmRSS:") << 10;
    }
    const std::string error_path = join_path(work_dir, errorFile);
    long outfile_size = get_file_size(outfile);
    int io_tick = 0;
    constexpr int io_check_interval = 16;
    constexpr int io_check_ms = 50;
    auto last_io_check = std::chrono::steady_clock::now();
    while (true) {
        bool check_io = should_check_io(io_tick, last_io_check, io_check_ms, io_check_interval);
        if (process_watch_iteration(pidApp, status, ruse, languageModel, mem_lmt, topmemory, ACflg, isspj,
                                    userfile, outfile, error_path.c_str(), solution_id, call_counter_local,
                                    last_error_size, outfile_size, check_io, record_syscall, debug_enabled,
                                    config, work_dir)) {
            break;
        }
    }
    add_usedtime(usedtime, ruse);
}

}  // namespace judge_watch_helpers
