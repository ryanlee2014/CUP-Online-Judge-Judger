#include "judge_client_report_case_helpers.h"

#include "judge_client_case_executor.h"

using namespace std;

void run_single_testcase(JudgeContext &ctx, const RunSingleTestcaseRequest &request,
                         JudgePaths &paths, string &global_work_dir, CaseExecutionState &state) {
    CaseExecutionInput input{ctx};
    input.runner_id = request.runner_id;
    input.solution_id = request.solution_id;
    input.limits.num_of_test = request.num_of_test;
    input.limits.memory_limit = request.memory_limit;
    input.limits.time_limit = request.time_limit;
    input.io.paths = &paths;
    input.io.usercode = request.usercode;
    input.io.global_work_dir = &global_work_dir;
    input.io.syscall_template = request.syscall_template;
    input.io.infile_pair = request.infile_pair;
    input.state = state;
    const CaseExecutionOutput output = execute_single_case(input);
    state = output;
}
