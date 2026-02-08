#include "test_common.h"
#include "../judge_client_watch_internal.h"

TEST(JudgeClientWatchSolutionBranches) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    TempDir tmp;
    std::string root = tmp.path.string();
    JudgeEnv env = make_env_with_home(root);
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(tmp.path);
    std::string user_path = (tmp.path / "user.out").string();
    std::string out_path = (tmp.path / "out.out").string();
    write_file(user_path, "1");
    write_file(out_path, "1");
    char infile[] = "in";
    int lang = 0;
    int p_id = 1;
    int pe = 0;
    double used = 0;
    double tl = 1.0;
    int mem = 1;
    RuntimeTestInputs runtime = make_runtime_test_inputs(false, false, true, false);

    int ac = ACCEPT;
    int top = STD_MB * 2;
    test_hooks::state().wait4_status = 0;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), runtime.config,
                   env, runtime.record_syscall, runtime.debug_enabled);
    EXPECT_EQ(ac, MEMORY_LIMIT_EXCEEDED);

    ac = ACCEPT;
    top = 0;
    write_file(tmp.path / "error.out", "Killed");
    write_file(std::filesystem::path("error.out"), "Killed");
    test_hooks::state().wait4_status = 1;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, 64,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), runtime.config,
                   env, runtime.record_syscall, runtime.debug_enabled);
    EXPECT_TRUE(ac == ACCEPT || ac == RUNTIME_ERROR);

    ac = ACCEPT;
    top = 0;
    write_file(user_path, std::string(3000, 'a'));
    write_file(out_path, "1");
    write_file(tmp.path / "error.out", "");
    test_hooks::state().wait4_status = 1;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, 64,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), runtime.config,
                   env, runtime.record_syscall, runtime.debug_enabled);
    EXPECT_EQ(ac, OUTPUT_LIMIT_EXCEEDED);

    ac = ACCEPT;
    top = 0;
    write_file(user_path, "1");
    test_hooks::state().wait4_status = (SIGXCPU << 8) | 1;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, 64,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), runtime.config,
                   env, runtime.record_syscall, runtime.debug_enabled);
    EXPECT_TRUE(ac != ACCEPT);

    ac = ACCEPT;
    top = 0;
    test_hooks::state().wait4_status = SIGXCPU;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, 64,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), runtime.config,
                   env, runtime.record_syscall, runtime.debug_enabled);
    EXPECT_TRUE(ac != ACCEPT);
    std::filesystem::current_path(old_cwd);
}

TEST(JudgeClientWatchSolutionKilledDebug) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    TempDir tmp;
    std::string root = tmp.path.string();
    JudgeEnv env = make_env_with_home(root);
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(tmp.path);
    std::string user_path = (tmp.path / "user.out").string();
    std::string out_path = (tmp.path / "out.out").string();
    write_file(user_path, "1");
    write_file(out_path, "1");
    write_file(tmp.path / "error.out", "Killed");
    char infile[] = "in";
    int lang = 0;
    int p_id = 1;
    int pe = 0;
    double used = 0;
    double tl = 1.0;
    int mem = 1024;
    int top = 0;
    int ac = ACCEPT;
    RuntimeTestInputs runtime = make_runtime_test_inputs(true, false, false, false);
    test_hooks::state().wait4_status = 1;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), runtime.config,
                   env, runtime.record_syscall, runtime.debug_enabled);
    std::filesystem::current_path(old_cwd);
}

TEST(JudgeClientWatchSolutionErrorAndExitStatus) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    TempDir tmp;
    std::string root = tmp.path.string();
    JudgeEnv env = make_env_with_home(root);
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(tmp.path);
    std::string user_path = (tmp.path / "user.out").string();
    std::string out_path = (tmp.path / "out.out").string();
    write_file(user_path, "1");
    write_file(out_path, "1");
    write_file(tmp.path / "error.out", "Killed");
    char infile[] = "in";
    int lang = 0;
    int p_id = 1;
    int pe = 0;
    double used = 0;
    double tl = 1.0;
    int mem = 64;
    int top = 0;
    int ac = ACCEPT;
    RuntimeTestInputs runtime = make_runtime_test_inputs(false, false, true, false);
    test_hooks::state().wait4_status = SIGKILL;
    watch_solution(1, infile, ac, 1, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), runtime.config,
                   env, runtime.record_syscall, runtime.debug_enabled);
    EXPECT_TRUE(read_file(tmp.path / "error.out").find("Runtime Error:") != std::string::npos);

    write_file(tmp.path / "error.out", "");
    ac = ACCEPT;
    top = 0;
    test_hooks::state().wait4_status = SIGALRM;
    watch_solution(1, infile, ac, 1, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), runtime.config,
                   env, runtime.record_syscall, runtime.debug_enabled);
    EXPECT_EQ(ac, TIME_LIMIT_EXCEEDED);

    write_file(tmp.path / "error.out", "");
    ac = ACCEPT;
    top = 0;
    test_hooks::state().wait4_status = SIGXFSZ;
    watch_solution(1, infile, ac, 1, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), runtime.config,
                   env, runtime.record_syscall, runtime.debug_enabled);
    EXPECT_EQ(ac, OUTPUT_LIMIT_EXCEEDED);

    write_file(tmp.path / "error.out", "");
    ac = ACCEPT;
    top = 0;
    test_hooks::state().wait4_status = SIGILL;
    watch_solution(1, infile, ac, 1, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), runtime.config,
                   env, runtime.record_syscall, runtime.debug_enabled);
    EXPECT_EQ(ac, RUNTIME_ERROR);
    std::filesystem::current_path(old_cwd);
}

TEST(JudgeClientWatchSolutionExitcodeDebug) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    TempDir tmp;
    std::string root = tmp.path.string();
    JudgeEnv env = make_env_with_home(root);
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(tmp.path);
    std::string user_path = (tmp.path / "user.out").string();
    std::string out_path = (tmp.path / "out.out").string();
    write_file(user_path, "1");
    write_file(out_path, "1");
    char infile[] = "in";
    int lang = 0;
    int p_id = 1;
    int pe = 0;
    double used = 0;
    double tl = 1.0;
    int mem = 64;
    int top = 0;
    int ac = ACCEPT;
    RuntimeTestInputs runtime = make_runtime_test_inputs(true, true, true, false);
    test_hooks::state().wait4_status = (SIGXFSZ << 8) | 1;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), runtime.config,
                   env, runtime.record_syscall, runtime.debug_enabled);
    EXPECT_EQ(ac, OUTPUT_LIMIT_EXCEEDED);
    std::filesystem::current_path(old_cwd);
}

TEST(JudgeClientWatchSolutionSignalDebug) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    TempDir tmp;
    std::string root = tmp.path.string();
    JudgeEnv env = make_env_with_home(root);
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(tmp.path);
    std::string user_path = (tmp.path / "user.out").string();
    std::string out_path = (tmp.path / "out.out").string();
    write_file(user_path, "1");
    write_file(out_path, "1");
    char infile[] = "in";
    int lang = 0;
    int p_id = 1;
    int pe = 0;
    double used = 0;
    double tl = 1.0;
    int mem = 64;
    int top = 0;
    int ac = ACCEPT;
    RuntimeTestInputs runtime = make_runtime_test_inputs(true, false, true, false);
    test_hooks::state().wait4_status = SIGXFSZ;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), runtime.config,
                   env, runtime.record_syscall, runtime.debug_enabled);
    EXPECT_EQ(ac, OUTPUT_LIMIT_EXCEEDED);
    std::filesystem::current_path(old_cwd);
}

TEST(JudgeClientWatchSolutionMemoryLimitDebug) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    TempDir tmp;
    std::string root = tmp.path.string();
    JudgeEnv env = make_env_with_home(root);
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(tmp.path);
    std::string user_path = (tmp.path / "user.out").string();
    std::string out_path = (tmp.path / "out.out").string();
    write_file(user_path, "1");
    write_file(out_path, "1");
    char infile[] = "in";
    int lang = 0;
    int p_id = 1;
    int pe = 0;
    double used = 0;
    double tl = 1.0;
    int mem = 1;
    int top = STD_MB * 2;
    int ac = ACCEPT;
    RuntimeTestInputs runtime = make_runtime_test_inputs(true, true, true, false);
    test_hooks::state().wait4_status = 1;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), runtime.config,
                   env, runtime.record_syscall, runtime.debug_enabled);
    std::filesystem::current_path(old_cwd);
}

TEST(JudgeClientWatchSolutionRuntimeErrorBranch) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    TempDir tmp;
    std::string root = tmp.path.string();
    JudgeEnv env = make_env_with_home(root);
    languageNameReader.loadJSON("{\"0\":\"c11\"}");
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(tmp.path);
    std::string user_path = (tmp.path / "user.out").string();
    std::string out_path = (tmp.path / "out.out").string();
    write_file(user_path, "1");
    write_file(out_path, "1");
    write_file(std::filesystem::path("error.out"), "oops");
    char infile[] = "in";
    int lang = 0;
    int p_id = 1;
    int pe = 0;
    double used = 0;
    double tl = 1.0;
    int mem = 64;
    int top = 0;
    int ac = ACCEPT;
    RuntimeTestInputs runtime = make_runtime_test_inputs(false, true, false, false);
    test_hooks::state().wait4_status = 1;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), runtime.config,
                   env, runtime.record_syscall, runtime.debug_enabled);
    EXPECT_EQ(ac, RUNTIME_ERROR);
    std::filesystem::current_path(old_cwd);
}

TEST(JudgeClientWatchSolutionOutputLimitPtrace) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    TempDir tmp;
    std::string root = tmp.path.string();
    JudgeEnv env = make_env_with_home(root);
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(tmp.path);
    std::string user_path = (tmp.path / "user.out").string();
    std::string out_path = (tmp.path / "out.out").string();
    write_file(out_path, "1");
    write_file(user_path, std::string(3000, 'a'));
    char infile[] = "in";
    int lang = 0;
    int p_id = 1;
    int pe = 0;
    double used = 0;
    double tl = 1.0;
    int mem = 64;
    int top = 0;
    int ac = ACCEPT;
    RuntimeTestInputs runtime = make_runtime_test_inputs(false, true, true, false);
    test_hooks::state().wait4_status = 1;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), runtime.config,
                   env, runtime.record_syscall, runtime.debug_enabled);
    EXPECT_EQ(ac, OUTPUT_LIMIT_EXCEEDED);
    std::filesystem::current_path(old_cwd);
}

TEST(JudgeClientWatchSolutionPtraceBranches) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    TempDir tmp;
    std::string root = tmp.path.string();
    JudgeEnv env = make_env_with_home(root);
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(tmp.path);
    std::string user_path = (tmp.path / "user.out").string();
    std::string out_path = (tmp.path / "out.out").string();
    write_file(user_path, "1");
    write_file(out_path, "1");
    write_file(tmp.path / "error.out", "");
    char infile[] = "in";
    int lang = 0;
    int p_id = 1;
    int pe = 0;
    double used = 0;
    double tl = 1.0;
    int mem = 64;
    int top = 0;
    test_hooks::state().ptrace_syscall = 12;
    std::fill(call_counter, call_counter + call_array_size, 0);
    test_hooks::state().wait4_statuses.push_back(0x7f);
    test_hooks::state().wait4_statuses.push_back(0);
    int ac = ACCEPT;
    RuntimeTestInputs runtime = make_runtime_test_inputs(false, true, true, true);
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), runtime.config,
                   env, runtime.record_syscall, runtime.debug_enabled);
    EXPECT_EQ(call_counter[12], 1);

    std::fill(call_counter, call_counter + call_array_size, 0);
    test_hooks::state().wait4_statuses.push_back(0x7f);
    test_hooks::state().wait4_statuses.push_back(0);
    ac = ACCEPT;
    runtime = make_runtime_test_inputs(false, true, true, false);
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), runtime.config,
                   env, runtime.record_syscall, runtime.debug_enabled);
    EXPECT_EQ(ac, RUNTIME_ERROR);
    std::filesystem::current_path(old_cwd);
}


TEST(JudgeClientWatchSolutionWithFileIdDebug) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    TempDir tmp;
    std::string root = tmp.path.string();
    JudgeEnv env = make_env_with_home(root);
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(tmp.path);
    std::string user_path = (tmp.path / "user1.out").string();
    std::string out_path = (tmp.path / "out1.out").string();
    write_file(user_path, "1");
    write_file(out_path, "1");
    write_file(tmp.path / "error1.out", "Killed");
    write_file(std::filesystem::path("error1.out"), "Killed");
    char infile[] = "in";
    int lang = 0;
    int p_id = 1;
    int pe = 0;
    double used = 0;
    double tl = 1.0;
    int mem = 1024;
    int top = 0;
    int ac = ACCEPT;
    int call_counter_local[call_array_size] = {};
    RuntimeTestInputs runtime = make_runtime_test_inputs(true, false, false, false);
    test_hooks::state().wait4_status = 1;
    watch_solution_with_file_id(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                                const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                                used, tl, p_id, pe, const_cast<char *>(root.c_str()), 1,
                                call_counter_local, runtime.config, env,
                                runtime.record_syscall, runtime.debug_enabled);
    std::filesystem::current_path(old_cwd);
}

TEST(JudgeClientWatchSolutionWithFileIdPtraceBranch) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    TempDir tmp;
    std::string root = tmp.path.string();
    JudgeEnv env = make_env_with_home(root);
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(tmp.path);
    std::string user_path = (tmp.path / "user1.out").string();
    std::string out_path = (tmp.path / "out1.out").string();
    write_file(user_path, "1");
    write_file(out_path, "1");
    char infile[] = "in";
    int lang = 0;
    int p_id = 1;
    int pe = 0;
    double used = 0;
    double tl = 1.0;
    int mem = 64;
    int top = 0;
    int ac = ACCEPT;
    int call_counter_local[call_array_size] = {};
    RuntimeTestInputs runtime = make_runtime_test_inputs(false, true, true, true);
    test_hooks::state().ptrace_syscall = 12;
    test_hooks::state().wait4_statuses.push_back(0x7f);
    test_hooks::state().wait4_statuses.push_back(0);
    watch_solution_with_file_id(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                                const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                                used, tl, p_id, pe, const_cast<char *>(root.c_str()), 1,
                                call_counter_local, runtime.config, env,
                                runtime.record_syscall, runtime.debug_enabled);
    EXPECT_EQ(call_counter_local[12], 1);
    std::filesystem::current_path(old_cwd);
}

TEST(JudgeClientWatchInternalHandlers) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    TempDir tmp;
    std::string root = tmp.path.string();
    JudgeEnv env = make_env_with_home(root);
    JudgeConfigSnapshot config;
    config.use_ptrace = 1;
    std::shared_ptr<Language> language_model = std::make_shared<Bash>();

    long last_error_size = 0;
    int ac = ACCEPT;
    write_file(tmp.path / "error.out", "");
    EXPECT_TRUE(!judge_watch_helpers::handle_error_conditions(language_model,
                                                              (tmp.path / "error.out").string().c_str(),
                                                              ac, 1, 1, last_error_size, false, config, env,
                                                              root.c_str()));

    write_file(tmp.path / "error.out", "Killed by signal");
    EXPECT_TRUE(judge_watch_helpers::handle_error_conditions(language_model,
                                                             (tmp.path / "error.out").string().c_str(),
                                                             ac, 1, 1, last_error_size, false, config, env,
                                                             root.c_str()));

    last_error_size = 0;
    ac = ACCEPT;
    config.all_test_mode = 0;
    write_file(tmp.path / "error.out", "runtime error");
    EXPECT_TRUE(judge_watch_helpers::handle_error_conditions(language_model,
                                                             (tmp.path / "error.out").string().c_str(),
                                                             ac, 1, 1, last_error_size, false, config, env,
                                                             root.c_str()));
    EXPECT_EQ(ac, RUNTIME_ERROR);
}

TEST(JudgeClientWatchInternalStatusHandlers) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    TempDir tmp;
    std::string root = tmp.path.string();
    JudgeConfigSnapshot config;
    config.use_ptrace = 1;
    std::shared_ptr<Language> language_model = std::make_shared<Bash>();
    int ac = ACCEPT;

    EXPECT_EQ(judge_watch_helpers::map_signal_to_ac(SIGXCPU), TIME_LIMIT_EXCEEDED);
    EXPECT_EQ(judge_watch_helpers::map_signal_to_ac(SIGXFSZ), OUTPUT_LIMIT_EXCEEDED);
    EXPECT_EQ(judge_watch_helpers::map_signal_to_ac(SIGILL), RUNTIME_ERROR);

    EXPECT_TRUE(!judge_watch_helpers::handle_exit_status(language_model, (0 << 8), ac, 1, false, config,
                                                         root.c_str()));
    EXPECT_TRUE(judge_watch_helpers::handle_exit_status(language_model, (SIGXFSZ << 8), ac, 1, false, config,
                                                        root.c_str()));
    EXPECT_EQ(ac, OUTPUT_LIMIT_EXCEEDED);

    ac = ACCEPT;
    EXPECT_TRUE(!judge_watch_helpers::handle_signal_status(0, ac, false, root.c_str()));
    EXPECT_TRUE(judge_watch_helpers::handle_signal_status(SIGXCPU, ac, true, root.c_str()));
    EXPECT_EQ(ac, TIME_LIMIT_EXCEEDED);

    ac = ACCEPT;
    EXPECT_TRUE(judge_watch_helpers::handle_signal_status(SIGSYS, ac, false, root.c_str()));
    EXPECT_EQ(ac, RUNTIME_ERROR);

    ac = ACCEPT;
    config.use_ptrace = 0;
    EXPECT_TRUE(judge_watch_helpers::handle_exit_status(language_model, (SIGTERM << 8), ac, 1, false, config,
                                                        root.c_str()));
    EXPECT_EQ(ac, RUNTIME_ERROR);
}

TEST(JudgeClientWatchInternalPtraceHandlers) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    TempDir tmp;
    std::string root = tmp.path.string();
    JudgeEnv env = make_env_with_home(root);
    JudgeConfigSnapshot config;
    config.use_ptrace = 1;
    int call_counter_local[call_array_size] = {};
    int ac = ACCEPT;

    call_counter_local[7] = 1;
    test_hooks::state().ptrace_syscall = 7;
    judge_watch_helpers::handle_ptrace_syscall(1, ac, 1, call_counter_local, false, config, env, root.c_str());
    EXPECT_EQ(ac, ACCEPT);

    call_counter_local[7] = 0;
    judge_watch_helpers::handle_ptrace_syscall(1, ac, 1, call_counter_local, true, config, env, root.c_str());
    EXPECT_EQ(call_counter_local[7], 1);

    call_counter_local[8] = 0;
    test_hooks::state().ptrace_syscall = 8;
    judge_watch_helpers::handle_ptrace_syscall(1, ac, 1, call_counter_local, false, config, env, root.c_str());
    EXPECT_EQ(ac, RUNTIME_ERROR);
}







