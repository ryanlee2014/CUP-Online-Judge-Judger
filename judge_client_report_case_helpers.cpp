#include "judge_client_report_case_helpers.h"

#include "judge_client_case_executor.h"

using namespace std;

void run_single_testcase(JudgeContext &ctx, int runner_id, int solution_id,
                         int num_of_test, int memoryLimit, double timeLimit, char *work_dir,
                         char *infile, char *outfile, char *userfile, char *usercode,
                         string &global_work_dir, int &topmemory, int &ACflg, int &PEflg,
                         int &pass_point, double &pass_rate, int &finalACflg, double &usedtime,
                         double &max_case_time, const int *syscall_template,
                         const pair<string, int> &infilePair) {
    CaseExecutionInput input{ctx};
    input.runner_id = runner_id;
    input.solution_id = solution_id;
    input.limits.num_of_test = num_of_test;
    input.limits.memory_limit = memoryLimit;
    input.limits.time_limit = timeLimit;
    input.io.work_dir = work_dir;
    input.io.infile = infile;
    input.io.outfile = outfile;
    input.io.userfile = userfile;
    input.io.usercode = usercode;
    input.io.global_work_dir = &global_work_dir;
    input.io.syscall_template = syscall_template;
    input.io.infile_pair = &infilePair;
    input.state.ACflg = ACflg;
    input.state.PEflg = PEflg;
    input.state.topmemory = topmemory;
    input.state.pass_point = pass_point;
    input.state.finalACflg = finalACflg;
    input.state.pass_rate = pass_rate;
    input.state.usedtime = usedtime;
    input.state.max_case_time = max_case_time;
    const CaseExecutionOutput output = execute_single_case(input);
    ACflg = output.ACflg;
    PEflg = output.PEflg;
    topmemory = output.topmemory;
    pass_point = output.pass_point;
    finalACflg = output.finalACflg;
    pass_rate = output.pass_rate;
    usedtime = output.usedtime;
    max_case_time = output.max_case_time;
}
