#include "judge_client_flow_runner.h"

#include <cstdio>
#include <vector>

#include "judge_client_report.h"
#include "judge_client_report_case_helpers.h"
#include "judge_client_run.h"

using namespace std;

namespace {
RunSingleTestcaseRequest build_case_request(int runner_id, int solution_id, const JudgeContext &ctx,
                                            const FlowState &state, const int *syscall_template_ptr,
                                            const pair<string, int> &infilePair) {
    RunSingleTestcaseRequest request;
    request.runner_id = runner_id;
    request.solution_id = solution_id;
    request.num_of_test = state.num_of_test;
    request.memory_limit = ctx.memory_limit;
    request.time_limit = ctx.time_limit;
    request.usercode = const_cast<char *>(ctx.usercode);
    request.syscall_template = syscall_template_ptr;
    request.infile_pair = &infilePair;
    return request;
}

CaseExecutionState build_execution_state(const FlowState &state) {
    CaseExecutionState execution_state;
    execution_state.ACflg = state.ACflg;
    execution_state.PEflg = state.PEflg;
    execution_state.topmemory = state.topmemory;
    execution_state.pass_point = state.pass_point;
    execution_state.finalACflg = state.finalACflg;
    execution_state.pass_rate = state.pass_rate;
    execution_state.usedtime = state.usedtime;
    execution_state.max_case_time = state.max_case_time;
    return execution_state;
}

void sync_case_state_back(const CaseExecutionState &execution_state, const JudgePaths &paths, FlowState &state) {
    state.ACflg = execution_state.ACflg;
    state.PEflg = execution_state.PEflg;
    state.topmemory = execution_state.topmemory;
    state.pass_point = execution_state.pass_point;
    state.finalACflg = execution_state.finalACflg;
    state.pass_rate = execution_state.pass_rate;
    state.usedtime = execution_state.usedtime;
    state.max_case_time = execution_state.max_case_time;
    snprintf(state.infile, BUFFER_SIZE, "%s", paths.infile.c_str());
    snprintf(state.outfile, BUFFER_SIZE, "%s", paths.outfile.c_str());
    snprintf(state.userfile, BUFFER_SIZE, "%s", paths.userfile.c_str());
}

void run_cases_parallel(int runner_id, JudgeContext &ctx, FlowState &state,
                        vector<pair<string, int>> &inFileList, const int *syscall_template_ptr) {
    auto r = runParallelJudge(runner_id, ctx.lang, state.work_dir, ctx.usercode, ctx.time_limit, state.usedtime,
                              ctx.memory_limit, inFileList, state.ACflg, ctx.special_judge,
                              state.global_work_dir,
                              ctx.submission, ctx.config, ctx.env,
                              ctx.flags.record_call != 0, ctx.flags.debug != 0, syscall_template_ptr,
                              ctx.language_factory, ctx.compare_factory);
    apply_parallel_result(r, state.num_of_test, ctx.time_limit, ctx.memory_limit, state.finalACflg,
                          state.ACflg, state.topmemory, state.usedtime,
                          state.max_case_time, state.pass_point, state.pass_rate, ctx.sender);
}

void maybe_run_case(JudgeContext &ctx, int runner_id, int solution_id, FlowState &state,
                    const int *syscall_template_ptr, pair<string, int> &infilePair) {
    if (state.ACflg <= PRESENTATION_ERROR) {
        RunSingleTestcaseRequest request =
            build_case_request(runner_id, solution_id, ctx, state, syscall_template_ptr, infilePair);
        JudgePaths paths;
        paths.work_dir = state.work_dir;
        CaseExecutionState execution_state = build_execution_state(state);
        run_single_testcase(ctx, request, paths, state.global_work_dir, execution_state);
        sync_case_state_back(execution_state, paths, state);
    }
}

void report_progress(JudgeContext &ctx, FlowState &state) {
    send_progress_update(state.usedtime,
                         state.topmemory,
                         ctx.time_limit,
                         ctx.memory_limit,
                         state.pass_point,
                         calculate_pass_rate(state.pass_rate, state.num_of_test), ctx.sender);
}

void run_cases_sequential(int runner_id, int solution_id, JudgeContext &ctx, FlowState &state,
                          vector<pair<string, int>> &inFileList, const int *syscall_template_ptr) {
    for (auto &infilePair: inFileList) {
        if (!should_continue_cases(state.ACflg, ctx.config)) {
            break;
        }
        maybe_run_case(ctx, runner_id, solution_id, state, syscall_template_ptr, infilePair);
        report_progress(ctx, state);
    }
}
}  // namespace

void run_cases(int runner_id, int solution_id, JudgeContext &ctx, FlowState &state,
               vector<pair<string, int>> &inFileList,
               const int *syscall_template_ptr) {
    if (ctx.config.enable_parallel && ctx.language_model->supportParallel()) {
        run_cases_parallel(runner_id, ctx, state, inFileList, syscall_template_ptr);
        return;
    }
    run_cases_sequential(runner_id, solution_id, ctx, state, inFileList, syscall_template_ptr);
}
