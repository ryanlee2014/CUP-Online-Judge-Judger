#include "test_common.h"
#include "../judge_client_runtime_bridge.h"

TEST(RealProjectFlow) {
    TempDir tmp;
    auto fixture_root = std::filesystem::path(__FILE__).parent_path()
                        / "fixtures" / "real_project";
    copy_tree(fixture_root, tmp.path);

    ConfigInfo cfg;
    auto config_path = tmp.path / "etc" / "config.json";
    auto config_str = config_path.string();
    cfg.readFromFile(config_str);
    EXPECT_EQ(cfg.getHostname(), "localhost");
    EXPECT_EQ(cfg.getUserName(), "u");
    EXPECT_EQ(cfg.getPassword(), "p");
    EXPECT_EQ(cfg.getDbName(), "jol");
    EXPECT_EQ(cfg.getPort(), 3306);
    EXPECT_EQ(cfg.getJavaTimeBonus(), 2);
    EXPECT_EQ(cfg.getJavaMemoryBonus(), 0);
    EXPECT_EQ(cfg.getAllTestMode(), 1);
    EXPECT_EQ(cfg.getFullDiff(), 0);
    EXPECT_EQ(cfg.getShareMemoryRun(), 0);
    EXPECT_EQ(cfg.getUseMaxTime(), 1);
    EXPECT_EQ(cfg.getUsePtrace(), 0);

    SubmissionInfo info;
    auto submission_path = tmp.path / "submission" / "1001.json";
    auto submission_str = submission_path.string();
    info.readFromFile(submission_str);
    EXPECT_EQ(info.getLanguage(), 0);
    EXPECT_EQ(info.getUserId(), "u");
    EXPECT_EQ(info.getProblemId(), 1001);
    EXPECT_EQ(info.getSpecialJudge(), false);
    EXPECT_EQ(info.getMemoryLimit(), 64);
    EXPECT_EQ(info.getTimeLimit(), 1.0);
    EXPECT_EQ(info.getSource(), "code");
    EXPECT_EQ(info.getSolutionId(), 1);

    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(tmp.path);
    auto ans = tmp.path / "data" / "1001" / "1.out";
    auto user = tmp.path / "run1" / "user.out";
    auto user_pe = tmp.path / "run1" / "user_pe.out";
    auto user_wa = tmp.path / "run1" / "user_wa.out";
    normalize_file_eol(ans);
    normalize_file_eol(user);
    normalize_file_eol(user_pe);
    normalize_file_eol(user_wa);
    int res_ac = compare_zoj(ans.string().c_str(), user.string().c_str(), 0, 0);
    EXPECT_EQ(res_ac, ACCEPT);
    int res_pe = compare_zoj(ans.string().c_str(), user_pe.string().c_str(), 0, 0);
    EXPECT_TRUE(res_pe == PRESENTATION_ERROR || res_pe == ACCEPT);
    int res_wa = compare_zoj(ans.string().c_str(), user_wa.string().c_str(), 0, 0);
    EXPECT_EQ(res_wa, WRONG_ANSWER);
    std::filesystem::current_path(old_cwd);
}

TEST(WSJudgedDebug) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard_after_reset;
    const char *argv[] = {"wsjudged", "1", "2", "/tmp", "DEBUG"};
    EXPECT_EQ(wsjudged_main(5, argv), 0);
    EXPECT_EQ(test_hooks::state().last_exec_path, "/usr/bin/judge_client");
}

TEST(JudgeClientInitParameters) {
    ScopedGlobalRuntimeGuard runtime_guard;
    int sid = 0;
    int rid = 0;
    std::string jid;
    const char *argv0[] = {"judge_client"};
    expect_exit([&]() {
        init_parameters(1, const_cast<char **>(argv0), sid, rid, jid);
    }, 1);
    TempDir tmp;
    std::string root = tmp.path.string();
    const char *argv1[] = {
        "judge_client",
        "-solution_id", "3",
        "-runner_id", "2",
        "-dir", root.c_str(),
        "-judger_id", "jid",
        "-language", "cpp",
        "-no_record",
        "-record",
        "-admin",
        "-no-sim",
        "-no-mysql",
        "-stdin",
        "DEBUG",
    };
    int argc1 = sizeof(argv1) / sizeof(argv1[0]);
    init_parameters(argc1, const_cast<char **>(argv1), sid, rid, jid);
    EXPECT_EQ(sid, 3);
    EXPECT_EQ(rid, 2);
    EXPECT_EQ(jid, "jid");
    EXPECT_TRUE(DEBUG);
    EXPECT_TRUE(NO_RECORD);
    EXPECT_TRUE(record_call);
    EXPECT_TRUE(admin);
    EXPECT_TRUE(no_sim);
    EXPECT_TRUE(!MYSQL_MODE);
    EXPECT_TRUE(READ_FROM_STDIN);
    EXPECT_EQ(std::string(oj_home), root);
    const char *argv2[] = {"judge_client", "1", "2", root.c_str(), "x", "DEBUG"};
    int argc2 = sizeof(argv2) / sizeof(argv2[0]);
    init_parameters(argc2, const_cast<char **>(argv2), sid, rid, jid);
    EXPECT_EQ(sid, 1);
    EXPECT_EQ(rid, 2);
    EXPECT_TRUE(NO_RECORD);
}

TEST(JudgeClientInitParametersOldPathRecordCall) {
    ScopedGlobalRuntimeGuard runtime_guard;
    int sid = 0;
    int rid = 0;
    std::string jid;
    TempDir tmp;
    std::string root = tmp.path.string();
    const char *argv1[] = {"judge_client", "1", "2", root.c_str(), "x", "cpp"};
    int argc1 = sizeof(argv1) / sizeof(argv1[0]);
    init_parameters(argc1, const_cast<char **>(argv1), sid, rid, jid);
    EXPECT_EQ(sid, 1);
    EXPECT_EQ(rid, 2);
    EXPECT_TRUE(record_call);
    EXPECT_EQ(std::string(oj_home), root);
    EXPECT_EQ(std::string(LANG_NAME), "cpp");
    const char *argv2[] = {"judge_client", "1", "2"};
    int argc2 = sizeof(argv2) / sizeof(argv2[0]);
    init_parameters(argc2, const_cast<char **>(argv2), sid, rid, jid);
    EXPECT_EQ(std::string(oj_home), "/home/judge");
}

TEST(JudgeClientPrintCallArray) {
    TempDir tmp;
    std::string root = tmp.path.string();
    const char *argv[] = {
        "judge_client",
        "-solution_id", "1",
        "-runner_id", "1",
        "-dir", root.c_str(),
        "-judger_id", "jid",
        "-language", "c11",
        "-no-mysql",
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    int sid = 0;
    int rid = 0;
    std::string jid;
    init_parameters(argc, const_cast<char **>(argv), sid, rid, jid);
    std::fill(call_counter, call_counter + call_array_size, 0);
    call_counter[1] = 1;
    call_counter[5] = 1;
    auto out_path = tmp.path / "call_array.txt";
    int stdout_fd = dup(fileno(stdout));
    FILE *fp = freopen(out_path.string().c_str(), "w", stdout);
    if (fp) {
        print_call_array();
        fflush(stdout);
    }
    if (stdout_fd >= 0) {
        dup2(stdout_fd, fileno(stdout));
        close(stdout_fd);
    }
    std::string content = read_file(out_path);
    EXPECT_TRUE(content.find("LANG_c11V") != std::string::npos);
}

TEST(JudgeClientCompileDebug) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    TempDir tmp;
    write_basic_config(tmp.path, false, false);
    (void)make_env_with_home(tmp.path.string());
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    auto work_dir = tmp.path / "run1";
    std::filesystem::create_directories(work_dir);
    auto work_str = work_dir.string();
    test_hooks::state().fork_results.push_back(0);
    JudgeEnv env = capture_env();
    RuntimeTestInputs runtime = make_runtime_test_inputs(true, true, true, false);
    expect_exit([&]() { compile(0, work_str.data(), env, runtime.config, runtime.debug_enabled); }, 0);

    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard_after_reset;
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    (void)make_env_with_home(tmp.path.string());
    test_hooks::state().compile_result = 0;
    test_hooks::state().fork_results.push_back(123);
    env = capture_env();
    runtime = make_runtime_test_inputs(false, true, true, false);
    EXPECT_EQ(compile(0, work_str.data(), env, runtime.config, runtime.debug_enabled), 0);
}

TEST(JudgeClientCompileChildParent) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    TempDir tmp;
    write_basic_config(tmp.path, false, false);
    (void)make_env_with_home(tmp.path.string());
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    auto work_dir = tmp.path / "run1";
    std::filesystem::create_directories(work_dir);
    auto work_str = work_dir.string();
    test_hooks::state().fork_results.push_back(0);
    JudgeEnv env = capture_env();
    RuntimeTestInputs runtime = make_runtime_test_inputs(false, true, true, false);
    expect_exit([&]() { compile(0, work_str.data(), env, runtime.config, runtime.debug_enabled); }, 0);
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard_after_reset;
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    (void)make_env_with_home(tmp.path.string());
    test_hooks::state().compile_result = 0;
    env = capture_env();
    runtime = make_runtime_test_inputs(false, true, true, false);
    EXPECT_EQ(compile(0, work_str.data(), env, runtime.config, runtime.debug_enabled), 0);
}

TEST(JudgeClientRunSolutionPaths) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    TempDir tmp;
    auto old_cwd = std::filesystem::current_path();
    (void)make_env_with_home(tmp.path.string());
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    write_file(tmp.path / "data.in", "1");
    write_file(tmp.path / "data1.in", "1");
    int lang = 0;
    double tl = 1.0;
    double used = 0.0;
    int mem = 64;
    RuntimeTestInputs runtime = make_runtime_test_inputs(false, false, true, false);
    auto work = tmp.path.string();
    int stdout_fd = dup(fileno(stdout));
    int stderr_fd = dup(fileno(stderr));
    expect_exit([&]() { run_solution(lang, work.data(), tl, used, mem, runtime.config); }, 0);
    std::filesystem::current_path(old_cwd);
    if (stdout_fd >= 0) {
        fflush(stdout);
        dup2(stdout_fd, fileno(stdout));
        close(stdout_fd);
    }
    if (stderr_fd >= 0) {
        fflush(stderr);
        dup2(stderr_fd, fileno(stderr));
        close(stderr_fd);
    }
    runtime = make_runtime_test_inputs(false, true, false, false);
    stdout_fd = dup(fileno(stdout));
    stderr_fd = dup(fileno(stderr));
    expect_exit([&]() { run_solution_parallel(lang, work.data(), tl, used, mem, 1, runtime.config); }, 0);
    std::filesystem::current_path(old_cwd);
    if (stdout_fd >= 0) {
        fflush(stdout);
        dup2(stdout_fd, fileno(stdout));
        close(stdout_fd);
    }
    if (stderr_fd >= 0) {
        fflush(stderr);
        dup2(stderr_fd, fileno(stderr));
        close(stderr_fd);
    }
}

TEST(JudgeClientRunSolutionNonAllTest) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    TempDir tmp;
    auto old_cwd = std::filesystem::current_path();
    (void)make_env_with_home(tmp.path.string());
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    write_file(tmp.path / "data.in", "1");
    int lang = 0;
    double tl = 1.0;
    double used = 500.0;
    int mem = 64;
    RuntimeTestInputs runtime = make_runtime_test_inputs(false, false, false, false);
    auto work = tmp.path.string();
    int stdout_fd = dup(fileno(stdout));
    int stderr_fd = dup(fileno(stderr));
    expect_exit([&]() { run_solution(lang, work.data(), tl, used, mem, runtime.config); }, 0);
    std::filesystem::current_path(old_cwd);
    if (stdout_fd >= 0) {
        fflush(stdout);
        dup2(stdout_fd, fileno(stdout));
        close(stdout_fd);
    }
    if (stderr_fd >= 0) {
        fflush(stderr);
        dup2(stderr_fd, fileno(stderr));
        close(stderr_fd);
    }
}

TEST(JudgeClientRunSolutionPtrace) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    TempDir tmp;
    auto old_cwd = std::filesystem::current_path();
    (void)make_env_with_home(tmp.path.string());
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    write_file(tmp.path / "data.in", "1");
    int lang = 0;
    double tl = 1.0;
    double used = 0.0;
    int mem = 64;
    RuntimeTestInputs runtime = make_runtime_test_inputs(false, true, true, false);
    auto work = tmp.path.string();
    int stdout_fd = dup(fileno(stdout));
    int stderr_fd = dup(fileno(stderr));
    expect_exit([&]() { run_solution(lang, work.data(), tl, used, mem, runtime.config); }, 0);
    std::filesystem::current_path(old_cwd);
    if (stdout_fd >= 0) {
        fflush(stdout);
        dup2(stdout_fd, fileno(stdout));
        close(stdout_fd);
    }
    if (stderr_fd >= 0) {
        fflush(stderr);
        dup2(stderr_fd, fileno(stderr));
        close(stderr_fd);
    }
}


TEST(WSJudgedBasics) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    const char *argv1[] = {"wsjudged"};
    EXPECT_EQ(wsjudged_main(1, argv1), 1);
    const char *argv2[] = {"wsjudged", "1", "2", "/tmp"};
    EXPECT_EQ(wsjudged_main(4, argv2), 0);
}

TEST(WSJudgedDebugArg) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    const char *argv[] = {"wsjudged", "1", "2", "/tmp", "debug"};
    EXPECT_EQ(wsjudged_main(5, argv), 0);
    EXPECT_EQ(test_hooks::state().last_exec_path, "/usr/bin/judge_client");
}

TEST(BootstrapRuntimeBridgeCapture) {
    ScopedGlobalRuntimeGuard runtime_guard;
    DEBUG = 1;
    record_call = 1;
    admin = true;
    no_sim = true;
    MYSQL_MODE = false;
    READ_FROM_STDIN = true;
    judger_number = 7;
    std::strcpy(oj_home, "/tmp/cup");

    auto runtime = capture_bootstrap_runtime_from_globals();
    EXPECT_TRUE(runtime.debug);
    EXPECT_TRUE(runtime.record_call);
    EXPECT_TRUE(runtime.admin);
    EXPECT_TRUE(runtime.no_sim);
    EXPECT_TRUE(runtime.disable_mysql);
    EXPECT_TRUE(runtime.read_from_stdin);
    EXPECT_TRUE(runtime.has_dir);
    EXPECT_EQ(runtime.dir, "/tmp/cup");
    EXPECT_TRUE(runtime.has_runner_id);
    EXPECT_EQ(runtime.runner_id, 7);
}


