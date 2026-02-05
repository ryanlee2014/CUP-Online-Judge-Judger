#ifndef JUDGE_CLIENT_REPORT_CASE_HELPERS_H
#define JUDGE_CLIENT_REPORT_CASE_HELPERS_H

#include <string>
#include <utility>

#include "judge_client_context.h"
#include "model/judge/language/Language.h"

void run_single_testcase(JudgeContext &ctx, int runner_id, int solution_id,
                         int num_of_test, int memoryLimit, double timeLimit, char *work_dir,
                         char *infile, char *outfile, char *userfile, char *usercode,
                         std::string &global_work_dir, int &topmemory, int &ACflg, int &PEflg,
                         int &pass_point, double &pass_rate, int &finalACflg, double &usedtime,
                         double &max_case_time, const int *syscall_template,
                         const std::pair<std::string, int> &infilePair);

#endif
