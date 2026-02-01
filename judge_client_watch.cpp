//
// Watch helpers extracted from judge_client.cpp
//

#include "judge_client_watch.h"

#include <cstdio>
#include <cstring>

#include "header/static_var.h"
#include "judge_client_context.h"
#include "judge_client_watch_helpers.h"

using namespace judge_watch_helpers;

extern int call_counter[call_array_size];

void watch_solution(pid_t pidApp, char *infile, int &ACflg, int isspj,
                    char *userfile, char *outfile, int solution_id, int lang,
                    int &topmemory, int mem_lmt, double &usedtime, double time_lmt, int &p_id,
                    int &PEflg, char *work_dir, const JudgeConfigSnapshot &config) {
    watch_solution_ex(pidApp, infile, ACflg, isspj, userfile, outfile, solution_id, lang, topmemory, mem_lmt, usedtime,
                      time_lmt, p_id, PEflg, work_dir, config, record_call != 0, DEBUG != 0);
}

void watch_solution_ex(pid_t pidApp, char *infile, int &ACflg, int isspj,
                       char *userfile, char *outfile, int solution_id, int lang,
                       int &topmemory, int mem_lmt, double &usedtime, double time_lmt, int &p_id,
                       int &PEflg, char *work_dir, const JudgeConfigSnapshot &config,
                       bool record_syscall, bool debug_enabled) {
    watch_solution_common(pidApp, infile, ACflg, isspj, userfile, outfile, solution_id, lang, topmemory, mem_lmt,
                          usedtime, "error.out", call_counter, config, work_dir, record_syscall, debug_enabled);
}

void watch_solution_with_file_id(pid_t pidApp, char *infile, int &ACflg, int isspj,
                                 char *userfile, char *outfile, int solution_id, int lang,
                                 int &topmemory, int mem_lmt, double &usedtime, double time_lmt, int &p_id,
                                 int &PEflg, char *work_dir, int file_id, int *call_counter_local,
                                 const JudgeConfigSnapshot &config) {
    watch_solution_with_file_id_ex(pidApp, infile, ACflg, isspj, userfile, outfile, solution_id, lang, topmemory,
                                   mem_lmt, usedtime, time_lmt, p_id, PEflg, work_dir, file_id, call_counter_local,
                                   config, record_call != 0, DEBUG != 0);
}

void watch_solution_with_file_id_ex(pid_t pidApp, char *infile, int &ACflg, int isspj,
                                    char *userfile, char *outfile, int solution_id, int lang,
                                    int &topmemory, int mem_lmt, double &usedtime, double time_lmt, int &p_id,
                                    int &PEflg, char *work_dir, int file_id, int *call_counter_local,
                                    const JudgeConfigSnapshot &config, bool record_syscall, bool debug_enabled) {
    char errorFile[BUFFER_SIZE];
    build_parallel_error_name(file_id, errorFile);
    watch_solution_common(pidApp, infile, ACflg, isspj, userfile, outfile, solution_id, lang, topmemory, mem_lmt,
                          usedtime, errorFile, call_counter_local, config, work_dir, record_syscall, debug_enabled);
}
