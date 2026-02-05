#include "judge_client_flow_runner.h"

#include <vector>

#include "judge_client_report.h"
#include "judge_client_run.h"

using namespace std;

namespace {
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
        run_single_testcase(ctx, runner_id, solution_id, state.num_of_test,
                            ctx.memory_limit, ctx.time_limit, state.work_dir, state.infile, state.outfile,
                            state.userfile, ctx.usercode,
                            state.global_work_dir, state.topmemory, state.ACflg, state.PEflg,
                            state.pass_point, state.pass_rate, state.finalACflg,
                            state.usedtime, state.max_case_time, syscall_template_ptr, infilePair);
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
