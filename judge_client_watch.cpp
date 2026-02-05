//
// Watch helpers extracted from judge_client.cpp
//

#include "judge_client_watch.h"

#include <cstdio>
#include <cstring>

#include "judge_client_context.h"
#include "judge_client_watch_helpers.h"

using namespace judge_watch_helpers;

extern int call_counter[call_array_size];

void watch_solution(pid_t pidApp, char *infile, int &ACflg, int isspj,
                    char *userfile, char *outfile, int solution_id, int lang,
                    int &topmemory, int mem_lmt, double &usedtime, double time_lmt, int &p_id,
                    int &PEflg, char *work_dir, const JudgeConfigSnapshot &config,
                    const JudgeEnv &env, bool record_syscall, bool debug_enabled) {
    watch_solution_ex(pidApp, infile, ACflg, isspj, userfile, outfile, solution_id, lang, topmemory, mem_lmt, usedtime,
                      time_lmt, p_id, PEflg, work_dir, config, env, record_syscall, debug_enabled);
}

void watch_solution_ex(pid_t pidApp, char *infile, int &ACflg, int isspj,
                       char *userfile, char *outfile, int solution_id, int lang,
                       int &topmemory, int mem_lmt, double &usedtime, double time_lmt, int &p_id,
                       int &PEflg, char *work_dir, const JudgeConfigSnapshot &config,
                       const JudgeEnv &env, bool record_syscall, bool debug_enabled) {
    WatchOptions options{&config, &env, work_dir, record_syscall, debug_enabled};
    WatchState state{ACflg, topmemory, usedtime};
    watch_solution_common(pidApp, infile, isspj, userfile, outfile, solution_id, lang, mem_lmt,
                          "error.out", call_counter, state, options);
}

void watch_solution_with_file_id(pid_t pidApp, char *infile, int &ACflg, int isspj,
                                 char *userfile, char *outfile, int solution_id, int lang,
                                 int &topmemory, int mem_lmt, double &usedtime, double time_lmt, int &p_id,
                                 int &PEflg, char *work_dir, int file_id, int *call_counter_local,
                                 const JudgeConfigSnapshot &config, const JudgeEnv &env,
                                 bool record_syscall, bool debug_enabled) {
    watch_solution_with_file_id_ex(pidApp, infile, ACflg, isspj, userfile, outfile, solution_id, lang, topmemory,
                                   mem_lmt, usedtime, time_lmt, p_id, PEflg, work_dir, file_id, call_counter_local,
                                   config, env, record_syscall, debug_enabled);
}

void watch_solution_with_file_id_ex(pid_t pidApp, char *infile, int &ACflg, int isspj,
                                    char *userfile, char *outfile, int solution_id, int lang,
                                    int &topmemory, int mem_lmt, double &usedtime, double time_lmt, int &p_id,
                                    int &PEflg, char *work_dir, int file_id, int *call_counter_local,
                                    const JudgeConfigSnapshot &config, const JudgeEnv &env,
                                    bool record_syscall, bool debug_enabled) {
    char errorFile[BUFFER_SIZE];
    build_parallel_error_name(file_id, errorFile);
    WatchOptions options{&config, &env, work_dir, record_syscall, debug_enabled};
    WatchState state{ACflg, topmemory, usedtime};
    watch_solution_common(pidApp, infile, isspj, userfile, outfile, solution_id, lang, mem_lmt,
                          errorFile, call_counter_local, state, options);
}
