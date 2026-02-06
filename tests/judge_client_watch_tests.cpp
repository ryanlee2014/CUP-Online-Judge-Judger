#include "test_common.h"

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
    use_ptrace = 0;
    RuntimeTestInputs runtime = make_runtime_test_inputs();

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
    DEBUG = 1;
    use_ptrace = 0;
    ALL_TEST_MODE = 0;
    RuntimeTestInputs runtime = make_runtime_test_inputs();
    test_hooks::state().wait4_status = 1;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), runtime.config,
                   env, runtime.record_syscall, runtime.debug_enabled);
    DEBUG = 0;
    ALL_TEST_MODE = 1;
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
    use_ptrace = 0;
    RuntimeTestInputs runtime = make_runtime_test_inputs();
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
    DEBUG = 1;
    use_ptrace = 1;
    RuntimeTestInputs runtime = make_runtime_test_inputs();
    test_hooks::state().wait4_status = (SIGXFSZ << 8) | 1;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), runtime.config,
                   env, runtime.record_syscall, runtime.debug_enabled);
    EXPECT_EQ(ac, OUTPUT_LIMIT_EXCEEDED);
    DEBUG = 0;
    use_ptrace = 0;
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
    DEBUG = 1;
    use_ptrace = 0;
    RuntimeTestInputs runtime = make_runtime_test_inputs();
    test_hooks::state().wait4_status = SIGXFSZ;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), runtime.config,
                   env, runtime.record_syscall, runtime.debug_enabled);
    EXPECT_EQ(ac, OUTPUT_LIMIT_EXCEEDED);
    DEBUG = 0;
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
    DEBUG = 1;
    use_ptrace = 1;
    RuntimeTestInputs runtime = make_runtime_test_inputs();
    test_hooks::state().wait4_status = 1;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), runtime.config,
                   env, runtime.record_syscall, runtime.debug_enabled);
    DEBUG = 0;
    use_ptrace = 0;
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
    ALL_TEST_MODE = 0;
    use_ptrace = 1;
    RuntimeTestInputs runtime = make_runtime_test_inputs();
    test_hooks::state().wait4_status = 1;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), runtime.config,
                   env, runtime.record_syscall, runtime.debug_enabled);
    EXPECT_EQ(ac, RUNTIME_ERROR);
    use_ptrace = 0;
    ALL_TEST_MODE = 1;
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
    use_ptrace = 1;
    RuntimeTestInputs runtime = make_runtime_test_inputs();
    test_hooks::state().wait4_status = 1;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), runtime.config,
                   env, runtime.record_syscall, runtime.debug_enabled);
    EXPECT_EQ(ac, OUTPUT_LIMIT_EXCEEDED);
    use_ptrace = 0;
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
    use_ptrace = 1;
    test_hooks::state().ptrace_syscall = 12;
    std::fill(call_counter, call_counter + call_array_size, 0);
    record_call = 1;
    test_hooks::state().wait4_statuses.push_back(0x7f);
    test_hooks::state().wait4_statuses.push_back(0);
    int ac = ACCEPT;
    RuntimeTestInputs runtime = make_runtime_test_inputs();
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), runtime.config,
                   env, runtime.record_syscall, runtime.debug_enabled);
    EXPECT_EQ(call_counter[12], 1);

    std::fill(call_counter, call_counter + call_array_size, 0);
    record_call = 0;
    test_hooks::state().wait4_statuses.push_back(0x7f);
    test_hooks::state().wait4_statuses.push_back(0);
    ac = ACCEPT;
    runtime = make_runtime_test_inputs();
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), runtime.config,
                   env, runtime.record_syscall, runtime.debug_enabled);
    EXPECT_EQ(ac, RUNTIME_ERROR);
    use_ptrace = 0;
    record_call = 0;
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
    DEBUG = 1;
    ALL_TEST_MODE = 0;
    use_ptrace = 0;
    RuntimeTestInputs runtime = make_runtime_test_inputs();
    test_hooks::state().wait4_status = 1;
    watch_solution_with_file_id(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                                const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                                used, tl, p_id, pe, const_cast<char *>(root.c_str()), 1,
                                call_counter_local, runtime.config, env,
                                runtime.record_syscall, runtime.debug_enabled);
    DEBUG = 0;
    ALL_TEST_MODE = 1;
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
    use_ptrace = 1;
    record_call = 1;
    RuntimeTestInputs runtime = make_runtime_test_inputs();
    test_hooks::state().ptrace_syscall = 12;
    test_hooks::state().wait4_statuses.push_back(0x7f);
    test_hooks::state().wait4_statuses.push_back(0);
    watch_solution_with_file_id(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                                const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                                used, tl, p_id, pe, const_cast<char *>(root.c_str()), 1,
                                call_counter_local, runtime.config, env,
                                runtime.record_syscall, runtime.debug_enabled);
    EXPECT_EQ(call_counter_local[12], 1);
    use_ptrace = 0;
    record_call = 0;
    std::filesystem::current_path(old_cwd);
}







