#ifndef JUDGE_CLIENT_WATCH_INTERNAL_H
#define JUDGE_CLIENT_WATCH_INTERNAL_H

#include <memory>

#include "judge_client_watch_helpers.h"
#include "judge_client_watch_metrics.h"

class Language;

namespace judge_watch_helpers {

bool handle_error_conditions(std::shared_ptr<Language> &languageModel, const char *errorFile, int &ACflg,
                             int solution_id, pid_t pidApp, long &last_error_size,
                             bool debug_enabled, const JudgeConfigSnapshot &config,
                             const JudgeEnv &env, const char *work_dir);
bool handle_output_limit(int isspj, const char *userfile, long outfile_size, int &ACflg, pid_t pidApp,
                         const JudgeConfigSnapshot &config);
int map_signal_to_ac(int signal_code);
bool handle_exit_status(std::shared_ptr<Language> &languageModel, int status, int &ACflg, pid_t pidApp,
                        bool debug_enabled, const JudgeConfigSnapshot &config, const char *work_dir);
bool handle_signal_status(int status, int &ACflg, bool debug_enabled, const char *work_dir);
void handle_ptrace_syscall(pid_t pidApp, int &ACflg, int solution_id, int *call_counter_local,
                           bool record_syscall, const JudgeConfigSnapshot &config,
                           const JudgeEnv &env, const char *work_dir);

WatchAction watch_phase_wait(WatchContext &ctx);
WatchAction watch_phase_resource(WatchContext &ctx);
WatchAction watch_phase_io(WatchContext &ctx, bool check_io);
WatchAction watch_phase_exit(WatchContext &ctx);
WatchAction watch_phase_signal(WatchContext &ctx);
WatchAction watch_phase_ptrace(WatchContext &ctx);
WatchAction run_watch_phase(WatchPhase phase, WatchContext &ctx, bool check_io);

}  // namespace judge_watch_helpers

#endif
