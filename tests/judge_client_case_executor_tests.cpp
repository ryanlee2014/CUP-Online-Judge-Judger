#include "test_common.h"
#include "../judge_client_case_executor.h"

namespace {
struct CaseFixture {
    TempDir tmp;
    JudgeContext ctx;
    JudgePaths paths;
    std::string global_work_dir;
    std::pair<std::string, int> infile_pair = {"1.in", 1};
    int syscall_template[call_array_size] = {};
    char usercode[16] = "code";
};

CaseFixture make_case_fixture(bool all_test_mode, bool use_max_time) {
    CaseFixture fixture;
    const std::string root = fixture.tmp.path.string();
    fixture.ctx.env = make_env_with_home(root);
    fixture.ctx.config = make_config_snapshot();
    fixture.ctx.config.all_test_mode = all_test_mode ? 1 : 0;
    fixture.ctx.config.use_max_time = use_max_time ? 1 : 0;
    fixture.ctx.config.use_ptrace = 0;
    fixture.ctx.flags.record_call = 0;
    fixture.ctx.flags.debug = 0;
    fixture.ctx.p_id = 1;
    fixture.ctx.lang = 0;
    fixture.ctx.special_judge = 0;
    fixture.paths.work_dir = root;
    fixture.global_work_dir = root + "/";
    std::filesystem::create_directories(fixture.tmp.path / "data" / "1");
    std::filesystem::create_directories(fixture.tmp.path / "run1");
    write_file(fixture.tmp.path / "data" / "1" / "1.in", "1");
    write_file(fixture.tmp.path / "data" / "1" / "1.out", "1");
    write_file(fixture.tmp.path / "run1" / "user.out", "1");
    write_file(fixture.tmp.path / "error.out", "");
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    return fixture;
}

CaseExecutionOutput run_case_with_template(CaseFixture &fixture, const CaseExecutionState &state,
                                           const int *syscall_template) {
    CaseExecutionInput input{
        fixture.ctx,
        1,
        1,
        {1, 64, 1.0},
        {&fixture.paths, fixture.usercode, &fixture.global_work_dir, syscall_template, &fixture.infile_pair},
        state};
    return execute_single_case(input);
}

CaseExecutionOutput run_case(CaseFixture &fixture, const CaseExecutionState &state) {
    return run_case_with_template(fixture, state, fixture.syscall_template);
}

struct CaseSpawnGuard {
    ~CaseSpawnGuard() {
        reset_case_spawn_function_for_test();
    }
};
}  // namespace

TEST(JudgeClientCaseExecutorAllTestModeKeepsFinalStatus) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    CaseFixture fixture = make_case_fixture(true, false);
    test_hooks::state().compare_result = WRONG_ANSWER;
    test_hooks::state().wait4_status = 0;

    CaseExecutionState state;
    state.ACflg = ACCEPT;
    state.finalACflg = ACCEPT;
    CaseExecutionOutput output = run_case(fixture, state);
    EXPECT_EQ(output.ACflg, ACCEPT);
    EXPECT_EQ(output.finalACflg, WRONG_ANSWER);
    EXPECT_EQ(output.pass_rate, 0);
    EXPECT_EQ(output.pass_point, 0);
}

TEST(JudgeClientCaseExecutorAcceptUpdatesPassCounters) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    CaseFixture fixture = make_case_fixture(false, false);
    test_hooks::state().compare_result = ACCEPT;
    test_hooks::state().wait4_status = 0;

    CaseExecutionState state;
    state.ACflg = ACCEPT;
    CaseExecutionOutput output = run_case(fixture, state);
    EXPECT_EQ(output.ACflg, ACCEPT);
    EXPECT_EQ(output.pass_point, 1);
}

TEST(JudgeClientCaseExecutorUseMaxTimeTracksCaseMax) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    CaseFixture fixture = make_case_fixture(false, true);
    test_hooks::state().compare_result = ACCEPT;
    test_hooks::state().wait4_status = 0;
    test_hooks::state().wait4_utime_ms = 37;

    CaseExecutionState state;
    state.ACflg = ACCEPT;
    state.max_case_time = 5;
    CaseExecutionOutput output = run_case(fixture, state);
    EXPECT_EQ(output.max_case_time, 37);
    EXPECT_EQ(output.usedtime, 0);
}

TEST(JudgeClientCaseExecutorClampTimeLimit) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    CaseFixture fixture = make_case_fixture(false, false);
    test_hooks::state().compare_result = ACCEPT;
    test_hooks::state().wait4_status = 0;

    CaseExecutionState state;
    state.ACflg = ACCEPT;
    state.usedtime = 1500;
    CaseExecutionOutput output = run_case(fixture, state);
    EXPECT_EQ(output.ACflg, TIME_LIMIT_EXCEEDED);
    EXPECT_EQ(static_cast<int>(output.usedtime), 1000);
    EXPECT_EQ(output.pass_point, 0);
}

TEST(JudgeClientCaseExecutorAllTestModePromotesFinalAcStatus) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    CaseFixture fixture = make_case_fixture(true, false);
    test_hooks::state().compare_result = ACCEPT;
    test_hooks::state().wait4_status = 0;

    CaseExecutionState state;
    state.ACflg = ACCEPT;
    state.finalACflg = ZERO;
    CaseExecutionOutput output = run_case(fixture, state);
    EXPECT_EQ(output.ACflg, ACCEPT);
    EXPECT_EQ(output.finalACflg, ACCEPT);
    EXPECT_EQ(output.pass_rate, 1);
}

TEST(JudgeClientCaseExecutorNullSyscallTemplateUsesInitManagerCounter) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    CaseFixture fixture = make_case_fixture(false, false);
    fixture.ctx.config.use_ptrace = 1;
    fixture.ctx.flags.record_call = 0;
    test_hooks::state().compare_result = ACCEPT;
    test_hooks::state().ptrace_syscall = 0;
    test_hooks::state().wait4_statuses.push_back(0x7f);
    test_hooks::state().wait4_statuses.push_back(0);

    CaseExecutionState state;
    state.ACflg = ACCEPT;
    CaseExecutionOutput output = run_case_with_template(fixture, state, nullptr);
    EXPECT_EQ(output.ACflg, ACCEPT);
    EXPECT_EQ(output.pass_point, 1);
}

TEST(JudgeClientCaseExecutorChildPathSkipsParentWatchFlow) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    CaseSpawnGuard spawn_guard;
    CaseFixture fixture = make_case_fixture(false, false);
    set_case_spawn_function_for_test([](const std::function<void()> &) {
        return static_cast<pid_t>(CHILD_PROCESS);
    });
    CaseExecutionState state;
    state.ACflg = ACCEPT;
    CaseExecutionOutput output = run_case(fixture, state);
    EXPECT_EQ(output.ACflg, ACCEPT);
    EXPECT_EQ(output.pass_point, 1);
}
