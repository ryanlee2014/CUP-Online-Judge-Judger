#ifndef JUDGE_CLIENT_WATCH_HELPERS_H
#define JUDGE_CLIENT_WATCH_HELPERS_H

#include <chrono>
#include <memory>
#include <string>
#include <sys/resource.h>

#include "judge_client_context.h"
#include "model/judge/language/Language.h"

namespace judge_watch_helpers {

void build_parallel_error_name(int file_id, char *errorOutput);

enum class WatchAction {
    Continue,
    Stop
};

enum class WatchPhase {
    Wait,
    Resource,
    Io,
    ExitCode,
    Signal,
    Ptrace
};

struct WatchOptions {
    const JudgeConfigSnapshot *config = nullptr;
    const JudgeEnv *env = nullptr;
    const char *work_dir = nullptr;
    bool record_syscall = false;
    bool debug_enabled = false;
};

struct WatchState {
    int &ACflg;
    int &topmemory;
    double &usedtime;
    long last_error_size = -1;
    long outfile_size = 0;
    int io_tick = 0;
    std::chrono::steady_clock::time_point last_io_check{};
};

struct WatchContext {
    pid_t pidApp = 0;
    int status = 0;
    struct rusage ruse{};
    int isspj = 0;
    int solution_id = 0;
    int lang = 0;
    int mem_lmt = 0;
    int *call_counter_local = nullptr;
    char *infile = nullptr;
    char *userfile = nullptr;
    char *outfile = nullptr;
    const char *errorFile = nullptr;
    std::string error_path;
    std::shared_ptr<Language> languageModel;
    WatchState *state = nullptr;
    const WatchOptions *options = nullptr;
};

void watch_solution_common(pid_t pidApp, char *infile, int isspj,
                           char *userfile, char *outfile, int solution_id, int lang,
                           int mem_lmt, const char *errorFile, int *call_counter_local,
                           WatchState &state, const WatchOptions &options);

}  // namespace judge_watch_helpers

#endif
