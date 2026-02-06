#include "judge_client_watch_helpers.h"

#include <chrono>
#include <iostream>
#include <memory>

#include "judge_client_watch_internal.h"
#include "judge_client_path_utils.h"
#include "model/judge/language/Language.h"

using namespace std;

namespace judge_watch_helpers {

void build_parallel_error_name(int file_id, char *errorOutput) {
    judge_path_utils::build_parallel_error_name(file_id, errorOutput);
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
    add_watch_usedtime(state.usedtime, context.ruse);
}

}  // namespace judge_watch_helpers
