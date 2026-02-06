#include "judge_client_flow_bootstrap.h"

#include <string>

#include "judge_client_compile.h"
#include "judge_client_flow_prep_helpers.h"
#include "judge_client_flow_socket_helpers.h"
#include "judge_client_report.h"
#include "judge_client_runtime_bridge.h"
#include "judge_client_util.h"
#include "library/judge_lib.h"
#include "model/websocket/WebSocketSender.h"

using namespace std;

void init_runtime_flags(int argc, char **argv, JudgeContext &ctx, int &solution_id, int &runner_id) {
    string judgerId;
    solution_id = DEFAULT_SOLUTION_ID;
    init_parameters(argc, argv, solution_id, runner_id, judgerId);
    ctx.judger_id = judgerId;
    judge_util_helpers::InitRuntimeConfig runtime = capture_bootstrap_runtime_from_globals();
    runtime.has_runner_id = true;
    runtime.runner_id = runner_id;
    apply_bootstrap_runtime_to_context(ctx, runtime);
    load_config(ctx);
    init_websocket_and_bundle(judgerId);
}

void init_sender(JudgeContext &ctx, int runner_id, FlowState &state) {
    prepare_environment(ctx, runner_id, state);
    if (!ctx.sender) {
        ctx.sender = [](const std::string &payload) { webSocket << payload; };
    }
}

void init_flow(int argc, char **argv, JudgeContext &ctx, FlowState &state,
               int &solution_id, int &runner_id) {
    init_runtime_flags(argc, argv, ctx, solution_id, runner_id);
    init_sender(ctx, runner_id, state);
}

void finalize_flow(const JudgeContext &ctx, const FlowState &state) {
    finalize_submission(ctx.judger_id, const_cast<char *>(state.work_dir));
    if (ctx.flags.debug) {
        write_log(ctx.env.oj_home.c_str(),
                  "result=%d", ctx.config.all_test_mode ? state.finalACflg : state.ACflg);
    }
    if (ctx.flags.record_call) {
        print_call_array();
    }
}
