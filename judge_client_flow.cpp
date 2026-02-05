#include "judge_client_flow.h"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "judge_client_compile.h"
#include "judge_client_flow_bootstrap.h"
#include "judge_client_flow_helpers.h"
#include "judge_client_flow_syscall_helpers.h"
#include "judge_client_flow_runner.h"
#include "judge_client_report.h"
#include "judge_client_util.h"
#include "library/judge_lib.h"
#include "model/websocket/WebSocketSender.h"

using namespace std;

int judge_client_main_impl(int argc, char **argv) {
    JudgeContext ctx;
    FlowState state;
    int runner_id = 0;
    init_flow(argc, argv, ctx, state, solution_id, runner_id);
    send_compiling_bundle(solution_id, ctx.env, ctx.sender);
    if (compile(ctx.lang, state.work_dir, ctx.env, ctx.config, ctx.flags.debug != 0) != COMPILED) {
        report_compile_error_and_exit(solution_id, ctx.judger_id, state.work_dir, ctx.env, ctx.sender);
    } else {
        umount(state.work_dir);
    }

    ctx.language_model->buildRuntime(state.work_dir);
    if (ctx.p_id <= TEST_RUN_PROBLEM) {
        handle_test_run(solution_id, ctx.lang, ctx.p_id, ctx.special_judge, ctx.time_limit, ctx.memory_limit,
                        state.work_dir, state.infile, state.outfile, state.userfile, state.topmemory,
                        state.ACflg, state.PEflg, state.usedtime, ctx.judger_id,
                        ctx.adapter, ctx.submission, ctx.language_model, ctx.config, ctx.env, ctx.sender,
                        ctx.flags.mysql_mode, ctx.flags.record_call != 0, ctx.flags.debug != 0);
    }
    int total_point = 0;
    vector<pair<string, int> > inFileList = getFileList(state.fullpath, isInFile);
    state.num_of_test = inFileList.size();
    total_point = state.num_of_test;
    send_running_bundle(solution_id, total_point, true, ctx.env, ctx.sender);
    vector<int> syscall_template(call_array_size);
    const int *syscall_template_ptr = nullptr;
    prepare_syscall_template(ctx, syscall_template, syscall_template_ptr);
    run_cases(runner_id, solution_id, ctx, state, inFileList, syscall_template_ptr);
    finalize_result_and_send(state.ACflg, state.finalACflg, state.PEflg, solution_id, ctx.lang, ctx.p_id,
                             ctx.language_model,
                             ctx.time_limit, state.topmemory,
                             state.pass_point, state.pass_rate, state.num_of_test, state.usedtime,
                             state.max_case_time, state.sim, state.sim_s_id, ctx.config,
                             state.work_dir, ctx.sender, ctx.flags.debug != 0);
    finalize_flow(ctx, state);
    return 0;
}
