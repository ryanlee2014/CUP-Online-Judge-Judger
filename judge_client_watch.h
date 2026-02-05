#ifndef JUDGE_CLIENT_WATCH_H
#define JUDGE_CLIENT_WATCH_H

#include <sys/types.h>

struct JudgeConfigSnapshot;
struct JudgeEnv;

void watch_solution(pid_t pidApp, char *infile, int &ACflg, int isspj,
                    char *userfile, char *outfile, int solution_id, int lang,
                    int &topmemory, int mem_lmt, double &usedtime, double time_lmt, int &p_id,
                    int &PEflg, char *work_dir, const JudgeConfigSnapshot &config,
                    const JudgeEnv &env, bool record_syscall, bool debug_enabled);

void watch_solution_ex(pid_t pidApp, char *infile, int &ACflg, int isspj,
                       char *userfile, char *outfile, int solution_id, int lang,
                       int &topmemory, int mem_lmt, double &usedtime, double time_lmt, int &p_id,
                       int &PEflg, char *work_dir, const JudgeConfigSnapshot &config,
                       const JudgeEnv &env,
                       bool record_syscall, bool debug_enabled);

void watch_solution_with_file_id(pid_t pidApp, char *infile, int &ACflg, int isspj,
                                 char *userfile, char *outfile, int solution_id, int lang,
                                 int &topmemory, int mem_lmt, double &usedtime, double time_lmt, int &p_id,
                                 int &PEflg, char *work_dir, int file_id, int *call_counter,
                                 const JudgeConfigSnapshot &config, const JudgeEnv &env,
                                 bool record_syscall, bool debug_enabled);

void watch_solution_with_file_id_ex(pid_t pidApp, char *infile, int &ACflg, int isspj,
                                    char *userfile, char *outfile, int solution_id, int lang,
                                    int &topmemory, int mem_lmt, double &usedtime, double time_lmt, int &p_id,
                                    int &PEflg, char *work_dir, int file_id, int *call_counter,
                                    const JudgeConfigSnapshot &config, const JudgeEnv &env,
                                    bool record_syscall, bool debug_enabled);

#endif
