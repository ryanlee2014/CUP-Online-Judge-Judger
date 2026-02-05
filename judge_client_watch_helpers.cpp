#include "judge_client_watch_helpers.h"

#include <algorithm>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstring>
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
#include "judge_client_path_utils.h"
#include "library/judge_lib.h"
#include "model/judge/language/Language.h"

using namespace std;

namespace judge_watch_helpers {

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
    judge_path_utils::build_parallel_error_name(file_id, errorOutput);
}

static bool handle_error_conditions(shared_ptr<Language> &languageModel, const char *errorFile, int &ACflg,
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

static WatchAction watch_phase_wait(WatchContext &ctx) {
    wait4(ctx.pidApp, &ctx.status, 0, &ctx.ruse);
    return WatchAction::Continue;
}

static WatchAction watch_phase_resource(WatchContext &ctx) {
    if (update_memory_and_check(ctx.languageModel, ctx.ruse, ctx.pidApp, ctx.mem_lmt, ctx.state->topmemory,
                                ctx.state->ACflg, ctx.options->debug_enabled, *ctx.options->config)) {
        return WatchAction::Stop;
    }
    if (WIFEXITED(ctx.status)) {
        return WatchAction::Stop;
    }
    return WatchAction::Continue;
}

static WatchAction watch_phase_io(WatchContext &ctx, bool check_io) {
    if (check_io) {
        if (handle_error_conditions(ctx.languageModel, ctx.error_path.c_str(), ctx.state->ACflg, ctx.solution_id,
                                    ctx.pidApp, ctx.state->last_error_size, ctx.options->debug_enabled,
                                    *ctx.options->config, *ctx.options->env, ctx.options->work_dir)) {
            return WatchAction::Stop;
        }
    }
    if (handle_output_limit(ctx.isspj, ctx.userfile, ctx.state->outfile_size, ctx.state->ACflg, ctx.pidApp,
                            *ctx.options->config)) {
        return WatchAction::Stop;
    }
    return WatchAction::Continue;
}

static WatchAction watch_phase_exit(WatchContext &ctx) {
    if (handle_exit_status(ctx.languageModel, ctx.status, ctx.state->ACflg, ctx.pidApp, ctx.options->debug_enabled,
                           *ctx.options->config, ctx.options->work_dir)) {
        return WatchAction::Stop;
    }
    return WatchAction::Continue;
}

static WatchAction watch_phase_signal(WatchContext &ctx) {
    if (handle_signal_status(ctx.status, ctx.state->ACflg, ctx.options->debug_enabled, ctx.options->work_dir)) {
        return WatchAction::Stop;
    }
    return WatchAction::Continue;
}

static WatchAction watch_phase_ptrace(WatchContext &ctx) {
    handle_ptrace_syscall(ctx.pidApp, ctx.state->ACflg, ctx.solution_id, ctx.call_counter_local,
                          ctx.options->record_syscall, *ctx.options->config, *ctx.options->env, ctx.options->work_dir);
    return WatchAction::Continue;
}

static WatchAction run_watch_phase(WatchPhase phase, WatchContext &ctx, bool check_io) {
    switch (phase) {
        case WatchPhase::Wait:
            return watch_phase_wait(ctx);
        case WatchPhase::Resource:
            return watch_phase_resource(ctx);
        case WatchPhase::Io:
            return watch_phase_io(ctx, check_io);
        case WatchPhase::ExitCode:
            return watch_phase_exit(ctx);
        case WatchPhase::Signal:
            return watch_phase_signal(ctx);
        case WatchPhase::Ptrace:
            return watch_phase_ptrace(ctx);
        default:
            return WatchAction::Continue;
    }
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

void watch_solution_common(pid_t pidApp, char *infile, int isspj,
                           char *userfile, char *outfile, int solution_id, int lang,
                           int mem_lmt, const char *errorFile, int *call_counter_local,
                           WatchState &state, const WatchOptions &options) {
    WatchContext context;
    context.pidApp = pidApp;
    context.isspj = isspj;
    context.solution_id = solution_id;
    context.lang = lang;
    context.mem_lmt = mem_lmt;
    context.call_counter_local = call_counter_local;
    context.infile = infile;
    context.userfile = userfile;
    context.outfile = outfile;
    context.errorFile = errorFile;
    context.state = &state;
    context.options = &options;
    context.languageModel = shared_ptr<Language>(getLanguageModel(lang));
    if (options.debug_enabled) {
        printf("pid=%d judging %s\n", pidApp, infile);
    }
    if (state.topmemory == 0) {
        state.topmemory = get_proc_status(pidApp, "VmRSS:") << 10;
    }
    context.error_path = judge_path_utils::join_path(options.work_dir, errorFile);
    state.outfile_size = get_file_size(outfile);
    state.io_tick = 0;
    constexpr int io_check_interval = 16;
    constexpr int io_check_ms = 50;
    constexpr WatchPhase kPhaseOrder[] = {
            WatchPhase::Wait,
            WatchPhase::Resource,
            WatchPhase::Io,
            WatchPhase::ExitCode,
            WatchPhase::Signal,
            WatchPhase::Ptrace};
    state.last_io_check = std::chrono::steady_clock::now();
    while (true) {
        bool check_io = should_check_io(state.io_tick, state.last_io_check, io_check_ms, io_check_interval);
        bool should_stop = false;
        for (WatchPhase phase : kPhaseOrder) {
            if (run_watch_phase(phase, context, check_io) == WatchAction::Stop) {
                should_stop = true;
                break;
            }
        }
        if (should_stop) {
            break;
        }
    }
    add_usedtime(state.usedtime, context.ruse);
}

}  // namespace judge_watch_helpers
