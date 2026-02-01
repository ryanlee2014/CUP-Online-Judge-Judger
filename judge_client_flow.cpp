#include "judge_client_flow.h"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "judge_client_compile.h"
#include "judge_client_context.h"
#include "judge_client_flow_helpers.h"
#include "judge_client_report.h"
#include "judge_client_run.h"
#include "judge_client_util.h"
#include "library/judge_lib.h"
#include "model/websocket/WebSocketSender.h"

using namespace std;

int judge_client_main_impl(int argc, char **argv) {
    string judgerId;
    JudgeContext ctx;
    FlowState state;
    solution_id = DEFAULT_SOLUTION_ID;
    int runner_id = 0;
    init_parameters(argc, argv, solution_id, runner_id, judgerId);
    ctx.judger_id = judgerId;
    load_config(ctx);
    initWebSocketConnection("localhost", 5100);
    bundle.setJudgerId(judgerId);
    prepare_environment(ctx, runner_id, state);

    if (!ctx.sender) {
        ctx.sender = [](const std::string &payload) { webSocket << payload; };
    }
    send_compiling_bundle(solution_id, ctx.sender);
    if (compile(ctx.lang, state.work_dir) != COMPILED) {
        report_compile_error_and_exit(solution_id, judgerId, state.work_dir, ctx.sender);
    } else {
        umount(state.work_dir);
    }

    ctx.language_model->buildRuntime(state.work_dir);
    if (ctx.p_id <= TEST_RUN_PROBLEM) {
        handle_test_run(solution_id, ctx.lang, ctx.p_id, ctx.special_judge, ctx.time_limit, ctx.memory_limit,
                        state.work_dir, state.infile, state.outfile, state.userfile, state.topmemory,
                        state.ACflg, state.PEflg, state.usedtime, judgerId,
                        ctx.adapter, ctx.submission, ctx.language_model, ctx.config, ctx.sender,
                        ctx.flags.record_call != 0, ctx.flags.debug != 0);
    }
    int total_point = 0;
    vector<pair<string, int> > inFileList = getFileList(state.fullpath, isInFile);
    state.num_of_test = inFileList.size();
    total_point = state.num_of_test;
    send_running_bundle(solution_id, total_point, true, ctx.sender);
    vector<int> syscall_template(call_array_size);
    const int *syscall_template_ptr = nullptr;
    prepare_syscall_template(ctx, syscall_template, syscall_template_ptr);
    if (ctx.config.enable_parallel && ctx.language_model->supportParallel()) {
        auto r = runParallelJudge(runner_id, ctx.lang, state.work_dir, ctx.usercode, ctx.time_limit, state.usedtime,
                                  ctx.memory_limit, inFileList, state.ACflg, ctx.special_judge,
                                  state.global_work_dir,
                                  ctx.submission, ctx.config, syscall_template_ptr);
        apply_parallel_result(r, state.num_of_test, ctx.time_limit, ctx.memory_limit, state.finalACflg,
                              state.ACflg, state.topmemory, state.usedtime,
                              state.max_case_time, state.pass_point, state.pass_rate, ctx.sender);
    } else {
        for (auto &infilePair: inFileList) {
            if (!should_continue_cases(state.ACflg, ctx.config)) {
                break;
            }

            if (state.ACflg <= PRESENTATION_ERROR) {
                run_single_testcase(ctx.lang, runner_id, solution_id, ctx.p_id, ctx.special_judge,
                                    state.num_of_test,
                                    ctx.memory_limit, ctx.time_limit, state.work_dir, state.infile, state.outfile,
                                    state.userfile,
                                    ctx.usercode,
                                    state.global_work_dir, state.topmemory, state.ACflg, state.PEflg,
                                    state.pass_point, state.pass_rate, state.finalACflg,
                                    state.usedtime, state.max_case_time, syscall_template_ptr, infilePair,
                                    ctx.config, ctx.flags.record_call != 0, ctx.flags.debug != 0);
            }
            send_progress_update(state.usedtime,
                                 state.topmemory,
                                 ctx.time_limit,
                                 ctx.memory_limit,
                                 state.pass_point,
                                 calculate_pass_rate(state.pass_rate, state.num_of_test), ctx.sender);
        }
    }
    finalize_result_and_send(state.ACflg, state.finalACflg, state.PEflg, solution_id, ctx.lang, ctx.p_id,
                             ctx.language_model,
                             ctx.time_limit, state.topmemory,
                             state.pass_point, state.pass_rate, state.num_of_test, state.usedtime,
                             state.max_case_time, state.sim, state.sim_s_id, ctx.config,
                             state.work_dir, ctx.sender, ctx.flags.debug != 0);
    finalize_submission(judgerId, state.work_dir);
    if (ctx.flags.debug) {
        write_log(oj_home, "result=%d", ctx.config.all_test_mode ? state.finalACflg : state.ACflg);
    }
    if (ctx.flags.record_call) {
        print_call_array();
    }
    return 0;
}
