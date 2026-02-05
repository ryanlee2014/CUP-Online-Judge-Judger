#include "judge_client_watch_internal.h"

#include <algorithm>
#include <csignal>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

#include "header/static_var.h"
#include "library/judge_lib.h"
#include "model/judge/language/Language.h"

using namespace std;

namespace judge_watch_helpers {

#ifdef __i386
#define REG_SYSCALL orig_eax
#else
#define REG_SYSCALL orig_rax
#endif

bool handle_error_conditions(shared_ptr<Language> &languageModel, const char *errorFile, int &ACflg,
                             int solution_id, pid_t pidApp, long &last_error_size,
                             bool debug_enabled, const JudgeConfigSnapshot &config,
                             const JudgeEnv &env, const char *work_dir) {
    long error_size = get_file_size(errorFile);
    bool has_error = error_size > 0;
    if (has_error && error_size != last_error_size) {
        fstream file(errorFile, ios::ate);
        stringstream buffer;
        buffer << file.rdbuf();
        string contents(buffer.str());
        if (debug_enabled) {
            cerr << "Catch error:" << endl;
            cerr << contents << endl;
        }
        if (contents.find("Killed") != std::string::npos) {
            write_log(env.oj_home.c_str(), contents.c_str());
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

bool handle_output_limit(int isspj, const char *userfile, long outfile_size, int &ACflg, pid_t pidApp,
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

int map_signal_to_ac(int signal_code) {
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

bool handle_exit_status(shared_ptr<Language> &languageModel, int status, int &ACflg, pid_t pidApp,
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

bool handle_signal_status(int status, int &ACflg, bool debug_enabled, const char *work_dir) {
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

void handle_ptrace_syscall(pid_t pidApp, int &ACflg, int solution_id, int *call_counter_local,
                           bool record_syscall, const JudgeConfigSnapshot &config,
                           const JudgeEnv &env, const char *work_dir) {
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
            write_log(env.oj_home.c_str(), _error.c_str());
            print_runtimeerror(_error.c_str(), work_dir);
            ptrace(PTRACE_KILL, pidApp, NULL, NULL);
        }
        ptrace(PTRACE_SYSCALL, pidApp, NULL, NULL);
    }
}

bool update_memory_and_check(shared_ptr<Language> &languageModel, const struct rusage &ruse, pid_t pidApp,
                             int mem_lmt, int &topmemory, int &ACflg,
                             bool debug_enabled, const JudgeConfigSnapshot &config) {
    int tempmemory = languageModel->getMemory(ruse, pidApp);
    topmemory = max(tempmemory, topmemory);
    if (topmemory > mem_lmt * STD_MB) {
        if (debug_enabled) {
            printf("out of memory %d\n", topmemory);
        }
        if (ACflg == ACCEPT) {
            ACflg = MEMORY_LIMIT_EXCEEDED;
        }
        if (config.use_ptrace) {
            ptrace(PTRACE_KILL, pidApp, NULL, NULL);
        }
        return true;
    }
    return false;
}

void add_usedtime(double &usedtime, const struct rusage &ruse) {
    usedtime += (ruse.ru_utime.tv_sec * 1000 + ruse.ru_utime.tv_usec / 1000);
    usedtime += (ruse.ru_stime.tv_sec * 1000 + ruse.ru_stime.tv_usec / 1000);
}

}  // namespace judge_watch_helpers
