#include "judge_client_watch_internal.h"

#include <algorithm>
#include <csignal>
#include <fstream>
#include <iostream>
#include <string>

#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

#include "library/judge_lib.h"
#include "model/judge/language/Language.h"

using namespace std;

namespace judge_watch_helpers {
namespace {
std::string read_file_tail(const char *path, long file_size, long max_bytes) {
    std::ifstream file(path, std::ios::binary);
    if (!file.good()) {
        return "";
    }
    long read_bytes = std::min(file_size, max_bytes);
    if (read_bytes <= 0) {
        return "";
    }
    file.seekg(file_size - read_bytes, std::ios::beg);
    std::string data(static_cast<size_t>(read_bytes), '\0');
    file.read(&data[0], read_bytes);
    data.resize(static_cast<size_t>(file.gcount()));
    return data;
}

std::string read_runtime_error_tail_if_changed(const char *errorFile, long error_size, long &last_error_size) {
    if (error_size <= 0 || error_size == last_error_size) {
        return "";
    }
    std::string contents = read_file_tail(errorFile, error_size, 4096);
    last_error_size = error_size;
    return contents;
}

bool classify_runtime_error(const std::string &contents) {
    return contents.find("Killed") != std::string::npos;
}

bool apply_runtime_error_action(bool is_killed_error, const std::string &contents, int &ACflg, pid_t pidApp,
                                bool has_error, bool debug_enabled, const JudgeConfigSnapshot &config,
                                const JudgeEnv &env, const char *work_dir, std::shared_ptr<Language> &languageModel) {
    if (is_killed_error) {
        write_log(env.oj_home.c_str(), contents.c_str());
        print_runtimeerror(contents.c_str(), work_dir);
        return true;
    }
    if (languageModel->gotErrorWhileRunning(has_error) && !config.all_test_mode) {
        ACflg = RUNTIME_ERROR;
        if (config.use_ptrace) {
            ptrace(PTRACE_KILL, pidApp, NULL, NULL);
        }
        return true;
    }
    if (debug_enabled && !contents.empty()) {
        cerr << "Catch error:" << endl;
        cerr << contents << endl;
    }
    return false;
}

bool update_output_limit_state(int isspj, const char *userfile, long outfile_size, int &ACflg) {
    if (isspj) {
        return false;
    }
    return get_file_size(userfile) > outfile_size * 2 + 1024 && ACflg != OUTPUT_LIMIT_EXCEEDED;
}

void update_ptrace_violation_state(int syscall_id, int solution_id, int &ACflg,
                                   const JudgeEnv &env, const char *work_dir) {
    ACflg = RUNTIME_ERROR;
    string error_text;
    error_text = string("Current Program use not allowed system call.\nSolution ID:") + to_string(solution_id) + "\n";
    error_text += string("Syscall ID:") + to_string(syscall_id) + "\n";
    write_log(env.oj_home.c_str(), error_text.c_str());
    print_runtimeerror(error_text.c_str(), work_dir);
}
}  // namespace

#ifdef __i386
#define REG_SYSCALL orig_eax
#else
#define REG_SYSCALL orig_rax
#endif

bool handle_error_conditions(shared_ptr<Language> &languageModel, const char *errorFile, int &ACflg,
                             int solution_id, pid_t pidApp, long &last_error_size,
                             bool debug_enabled, const JudgeConfigSnapshot &config,
                             const JudgeEnv &env, const char *work_dir) {
    (void)solution_id;
    long error_size = get_file_size(errorFile);
    bool has_error = error_size > 0;
    std::string contents = read_runtime_error_tail_if_changed(errorFile, error_size, last_error_size);
    bool killed_error = classify_runtime_error(contents);
    if (apply_runtime_error_action(killed_error, contents, ACflg, pidApp, has_error,
                                   debug_enabled, config, env, work_dir, languageModel)) {
        return true;
    }
    return false;
}

bool handle_output_limit(int isspj, const char *userfile, long outfile_size, int &ACflg, pid_t pidApp,
                         const JudgeConfigSnapshot &config) {
    if (update_output_limit_state(isspj, userfile, outfile_size, ACflg)) {
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
            update_ptrace_violation_state(static_cast<int>(reg.REG_SYSCALL), solution_id, ACflg, env, work_dir);
            ptrace(PTRACE_KILL, pidApp, NULL, NULL);
        }
        ptrace(PTRACE_SYSCALL, pidApp, NULL, NULL);
    }
}

}  // namespace judge_watch_helpers
