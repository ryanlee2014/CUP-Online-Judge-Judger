#ifndef JUDGE_CLIENT_WATCH_HELPERS_H
#define JUDGE_CLIENT_WATCH_HELPERS_H

#include <string>

#include "judge_client_context.h"

namespace judge_watch_helpers {

std::string join_path(const char *base, const char *name);
void build_parallel_error_name(int file_id, char *errorOutput);

void watch_solution_common(pid_t pidApp, char *infile, int &ACflg, int isspj,
                           char *userfile, char *outfile, int solution_id, int lang,
                           int &topmemory, int mem_lmt, double &usedtime,
                           const char *errorFile, int *call_counter_local,
                           const JudgeConfigSnapshot &config, const char *work_dir,
                           bool record_syscall, bool debug_enabled);

}  // namespace judge_watch_helpers

#endif
