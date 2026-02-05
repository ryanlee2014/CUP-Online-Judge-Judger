#ifndef JUDGE_CLIENT_WATCH_HELPERS_H
#define JUDGE_CLIENT_WATCH_HELPERS_H

#include <chrono>
#include <string>

#include "judge_client_context.h"

namespace judge_watch_helpers {

void build_parallel_error_name(int file_id, char *errorOutput);

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

void watch_solution_common(pid_t pidApp, char *infile, int isspj,
                           char *userfile, char *outfile, int solution_id, int lang,
                           int mem_lmt, const char *errorFile, int *call_counter_local,
                           WatchState &state, const WatchOptions &options);

}  // namespace judge_watch_helpers

#endif
