#include "judge_client_watch_internal.h"

#include <sys/wait.h>

namespace judge_watch_helpers {

WatchAction watch_phase_wait(WatchContext &ctx) {
    // In production, wait4() should block until the child changes state.
    // Under UNIT_TEST builds, wait4 is macro-overridden (see tests/test_hooks.h).
    // If a test stub returns an invalid result, bail out to avoid spinning forever.
#ifdef UNIT_TEST
    pid_t ret = wait4(ctx.pidApp, &ctx.status, 0, &ctx.ruse);
    if (ret < 0) {
        if (ctx.state && ctx.state->ACflg == ACCEPT) {
            ctx.state->ACflg = RUNTIME_ERROR;
        }
        return WatchAction::Stop;
    }
#else
    wait4(ctx.pidApp, &ctx.status, 0, &ctx.ruse);
#endif
    return WatchAction::Continue;
}

WatchAction watch_phase_resource(WatchContext &ctx) {
    WatchMetricsInput metrics;
    metrics.languageModel = ctx.languageModel;
    metrics.ruse = &ctx.ruse;
    metrics.pidApp = ctx.pidApp;
    metrics.mem_lmt = ctx.mem_lmt;
    metrics.topmemory = &ctx.state->topmemory;
    metrics.ACflg = &ctx.state->ACflg;
    metrics.debug_enabled = ctx.options->debug_enabled;
    metrics.config = ctx.options->config;
    update_watch_metrics(metrics);
    if (metrics.stop) {
        return WatchAction::Stop;
    }
#ifdef UNIT_TEST
    // If the child is stopped but ptrace is disabled, there is no way to make forward progress.
    // This can happen in UNIT_TEST when a stub returns a "stopped" wait status.
    if (WIFSTOPPED(ctx.status) && !ctx.options->config->use_ptrace) {
        if (ctx.state && ctx.state->ACflg == ACCEPT) {
            ctx.state->ACflg = RUNTIME_ERROR;
        }
        return WatchAction::Stop;
    }
#endif
    if (WIFEXITED(ctx.status)) {
        return WatchAction::Stop;
    }
    return WatchAction::Continue;
}

WatchAction watch_phase_io(WatchContext &ctx, bool check_io) {
    if (check_io) {
        if (handle_error_conditions(ctx.languageModel, ctx.error_path.c_str(), ctx.state->ACflg, ctx.solution_id,
                                    ctx.pidApp, ctx.state->last_error_size, ctx.options->debug_enabled,
                                    *ctx.options->config, *ctx.options->env, ctx.options->work_dir)) {
            return WatchAction::Stop;
        }
    }
    if (handle_output_limit(ctx.isspj, ctx.userfile, ctx.state->outfile_size, ctx.state->ACflg, ctx.pidApp,
                            *ctx.options->config)) {
        return WatchAction::Stop;
    }
    return WatchAction::Continue;
}

WatchAction watch_phase_exit(WatchContext &ctx) {
    if (handle_exit_status(ctx.languageModel, ctx.status, ctx.state->ACflg, ctx.pidApp, ctx.options->debug_enabled,
                           *ctx.options->config, ctx.options->work_dir)) {
        return WatchAction::Stop;
    }
    return WatchAction::Continue;
}

WatchAction watch_phase_signal(WatchContext &ctx) {
    if (handle_signal_status(ctx.status, ctx.state->ACflg, ctx.options->debug_enabled, ctx.options->work_dir)) {
        return WatchAction::Stop;
    }
    return WatchAction::Continue;
}

WatchAction watch_phase_ptrace(WatchContext &ctx) {
    handle_ptrace_syscall(ctx.pidApp, ctx.state->ACflg, ctx.solution_id, ctx.call_counter_local,
                          ctx.options->record_syscall, *ctx.options->config, *ctx.options->env, ctx.options->work_dir);
    return WatchAction::Continue;
}

WatchAction run_watch_phase(WatchPhase phase, WatchContext &ctx, bool check_io) {
    switch (phase) {
        case WatchPhase::Wait:
            return watch_phase_wait(ctx);
        case WatchPhase::Resource:
            return watch_phase_resource(ctx);
        case WatchPhase::Io:
            return watch_phase_io(ctx, check_io);
        case WatchPhase::ExitCode:
            return watch_phase_exit(ctx);
        case WatchPhase::Signal:
            return watch_phase_signal(ctx);
        case WatchPhase::Ptrace:
            return watch_phase_ptrace(ctx);
        default:
            return WatchAction::Continue;
    }
}

}  // namespace judge_watch_helpers
