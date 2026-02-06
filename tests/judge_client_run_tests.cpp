#include "test_common.h"
#include "../judge_client_run.h"

namespace {
struct EnvVarGuard {
    std::string key;
    std::string old_value;
    bool had_value = false;

    explicit EnvVarGuard(const char *name) : key(name) {
        const char *old = std::getenv(name);
        if (old) {
            had_value = true;
            old_value = old;
        }
    }

    ~EnvVarGuard() {
        if (had_value) {
            setenv(key.c_str(), old_value.c_str(), 1);
        } else {
            unsetenv(key.c_str());
        }
    }
};
}  // namespace

TEST(JudgeClientRunJudgeTaskBranches) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    TempDir tmp;
    std::string root = tmp.path.string();
    JudgeEnv env = make_env_with_home(root);
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    std::filesystem::create_directories(tmp.path / "data" / "1");
    write_file(tmp.path / "data" / "1" / "1.in", "");
    write_file(tmp.path / "data" / "1" / "1.out", "");
    write_file(tmp.path / "error0.out", "");
    std::string work = tmp.path.string();
    std::string global = work + "/";
    std::pair<std::string, int> infilePair("1.in", 1);
    char usercode[16] = "code";
    int ac = ACCEPT;
    bool record_syscall = false;
    bool debug_enabled = false;
    test_hooks::state().fork_results.push_back(0);
    int stdout_fd = dup(fileno(stdout));
    int stderr_fd = dup(fileno(stderr));
    expect_exit([&]() {
        runJudgeTask(1, 0, work.data(), infilePair, ac, 0, 1, 1.0, 0.0, 64,
                     1, usercode, 0, global, make_config_snapshot(), env, record_syscall, debug_enabled);
    }, 0);
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
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard_after_reset;
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    test_hooks::state().compare_result = ACCEPT;
    test_hooks::state().wait4_status = 0;
    stdout_fd = dup(fileno(stdout));
    stderr_fd = dup(fileno(stderr));
    JudgeResult res = runJudgeTask(1, 0, work.data(), infilePair, ac, 0, 1, 1.0,
                                   0.0, 64, 1, usercode, 0, global, make_config_snapshot(),
                                   env, record_syscall, debug_enabled);
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
    EXPECT_EQ(res.ACflg, ACCEPT);
}

TEST(JudgeClientRunJudgeTaskTimeLimitClamp) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    TempDir tmp;
    std::string root = tmp.path.string();
    JudgeEnv env = make_env_with_home(root);
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    std::filesystem::create_directories(tmp.path / "data" / "1");
    write_file(tmp.path / "data" / "1" / "1.in", "");
    write_file(tmp.path / "data" / "1" / "1.out", "");
    write_file(tmp.path / "error0.out", "");
    std::string work = tmp.path.string();
    std::string global = work + "/";
    std::pair<std::string, int> infilePair("1.in", 1);
    char usercode[16] = "code";
    int ac = ACCEPT;
    bool record_syscall = false;
    bool debug_enabled = false;
    test_hooks::state().compare_result = ACCEPT;
    test_hooks::state().wait4_status = (SIGALRM << 8) | 0x7f;
    test_hooks::state().wait4_utime_ms = 10;
    test_hooks::state().wait4_stime_ms = 0;
    JudgeResult res = runJudgeTask(1, 0, work.data(), infilePair, ac, 0, 1, 1.0,
                                   0.0, 64, 1, usercode, 0, global, make_config_snapshot(),
                                   env, record_syscall, debug_enabled);
    EXPECT_EQ(res.ACflg, TIME_LIMIT_EXCEEDED);
    EXPECT_EQ(static_cast<int>(res.usedTime), 1000);
}

TEST(JudgeClientRunParallelJudge) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    reset_globals_for_test();
    TempDir tmp;
    std::string root = tmp.path.string();
    JudgeEnv env = make_env_with_home(root);
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    std::filesystem::create_directories(tmp.path / "data" / "1");
    write_file(tmp.path / "data" / "1" / "1.in", "");
    write_file(tmp.path / "data" / "1" / "1.out", "");
    std::string work = tmp.path.string();
    std::string global = work + "/";
    SubmissionInfo submission;
    submission.setLanguage(0).setSolutionId(1).setTimeLimit(1).setMemoryLimit(64).setProblemId(1);
    std::vector<std::pair<std::string, int>> inFileList = {{"1.in", 1}, {"2.in", 1}};
    int ac = ACCEPT;
    bool record_syscall = false;
    bool debug_enabled = false;
    test_hooks::state().compare_result = ACCEPT;
    test_hooks::state().wait4_status = 0;
    auto res = runParallelJudge(1, 0, work.data(), const_cast<char *>("code"), 1, 0, 64,
                                inFileList, ac, 0, global, submission, make_config_snapshot(),
                                env, record_syscall, debug_enabled);
    EXPECT_TRUE(res.pass_point >= 0);
}

TEST(JudgeClientParallelBudgetBranches) {
    ParallelRunOptions opts;
    EXPECT_TRUE(compute_parallel_budget(opts) >= 1);

    std::vector<std::pair<std::string, int>> single_file = {{"1.in", 1}};
    opts.in_file_list = &single_file;
    opts.memory_limit = 1024;
    EXPECT_EQ(compute_parallel_budget(opts), 1);

    std::vector<std::pair<std::string, int>> files = {{"1.in", 1}, {"2.in", 1}, {"3.in", 1}};
    opts.in_file_list = &files;
    opts.memory_limit = 1024;

    {
        EnvVarGuard guard("JUDGE_PARALLEL_WORKERS");
        setenv("JUDGE_PARALLEL_WORKERS", "8", 1);
        EXPECT_EQ(compute_parallel_budget(opts), 3);
    }
    {
        EnvVarGuard guard("JUDGE_PARALLEL_WORKERS");
        setenv("JUDGE_PARALLEL_WORKERS", "8", 1);
        opts.memory_limit = 64;
        EXPECT_EQ(compute_parallel_budget(opts), 1);
    }
    {
        EnvVarGuard guard("JUDGE_PARALLEL_WORKERS");
        setenv("JUDGE_PARALLEL_WORKERS", "bad", 1);
        opts.memory_limit = 0;
        int workers = compute_parallel_budget(opts);
        EXPECT_TRUE(workers >= 1);
        EXPECT_TRUE(workers <= 3);
    }
}


TEST(JudgeClientMySQLSim) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    reset_globals_for_test();
    TempDir tmp;
    std::string root = tmp.path.string();
    (void)make_env_with_home(root);
    std::filesystem::create_directories(tmp.path / "etc");
    std::filesystem::create_directories(tmp.path / "data" / "100");
    write_file(tmp.path / "data" / "100" / "1.in", "");
    write_file(tmp.path / "data" / "100" / "1.out", "");
    write_file(tmp.path / "etc" / "language.json", "{\"1\":\"c11\"}");
    write_file(tmp.path / "etc" / "compile.json", "{\"1\":[\"/bin/true\"]}");
    write_file(tmp.path / "etc" / "config.json",
               "{\"hostname\":\"h\",\"username\":\"u\",\"password\":\"p\","
               "\"db_name\":\"d\",\"port\":1,\"java_time_bonus\":1,\"java_memory_bonus\":1,"
               "\"java_xms\":\"x\",\"java_xmx\":\"y\",\"sim_enable\":1,\"full_diff\":0,"
               "\"judger_name\":\"J\",\"shm_run\":1,\"use_max_time\":0,\"use_ptrace\":0,"
               "\"all_test_mode\":0,\"enable_parallel\":0}");
    test_hooks::state().mysql_problem_id = 100;
    test_hooks::state().mysql_language = 1;
    test_hooks::state().mysql_time_limit = 1.0;
    test_hooks::state().mysql_memory_limit = 64;
    test_hooks::state().compare_result = ACCEPT;
    test_hooks::state().compile_result = 0;
    test_hooks::state().wait4_status = 0;
    test_hooks::state().system_result = 0;
    const char *argv[] = {
        "judge_client",
        "-solution_id", "1",
        "-runner_id", "1",
        "-dir", root.c_str(),
        "-judger_id", "t1",
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    EXPECT_EQ(judge_client_main(argc, const_cast<char **>(argv)), 0);
}

TEST(JudgeClientCompileError) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    test_hooks::state().compile_result = 1;
    TempDir tmp;
    write_basic_config(tmp.path, false, false);
    write_submission(tmp.path, "t1", 1001, false);
    prepare_case_files(tmp.path, 1001);
    prepare_run_dir(tmp.path, 1, false);
    std::string root = tmp.path.string();
    const char *argv[] = {
        "judge_client",
        "-solution_id", "1",
        "-runner_id", "1",
        "-dir", root.c_str(),
        "-judger_id", "t1",
        "-no-mysql",
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    expect_exit([&]() { judge_client_main(argc, const_cast<char **>(argv)); }, 0);
}

TEST(JudgeClientMySQLStartFailure) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    test_hooks::state().mysql_start_ok = false;
    TempDir tmp;
    write_basic_config(tmp.path, false, false);
    std::string root = tmp.path.string();
    const char *argv[] = {
        "judge_client",
        "-solution_id", "1",
        "-runner_id", "1",
        "-dir", root.c_str(),
        "-judger_id", "tmysql",
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    expect_exit([&]() { judge_client_main(argc, const_cast<char **>(argv)); }, 1);
}

TEST(JudgeClientMySQLCompileError) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    test_hooks::state().compile_result = 1;
    test_hooks::state().mysql_start_ok = true;
    test_hooks::state().mysql_problem_id = 1001;
    test_hooks::state().mysql_language = 0;
    test_hooks::state().mysql_source = "code";
    TempDir tmp;
    write_basic_config(tmp.path, false, false);
    std::string root = tmp.path.string();
    const char *argv[] = {
        "judge_client",
        "-solution_id", "1",
        "-runner_id", "1",
        "-dir", root.c_str(),
        "-judger_id", "tmysql2",
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    expect_exit([&]() { judge_client_main(argc, const_cast<char **>(argv)); }, 0);
}

TEST(JudgeClientSequential) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    test_hooks::state().compile_result = 0;
    test_hooks::state().compare_result = ACCEPT;
    TempDir tmp;
    write_basic_config(tmp.path, false, false);
    write_submission(tmp.path, "t2", 1001, false);
    prepare_case_files(tmp.path, 1001);
    prepare_run_dir(tmp.path, 1, false);
    std::string root = tmp.path.string();
    const char *argv[] = {
        "judge_client",
        "-solution_id", "1",
        "-runner_id", "1",
        "-dir", root.c_str(),
        "-judger_id", "t2",
        "-no-mysql",
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    int rc = judge_client_main(argc, const_cast<char **>(argv));
    EXPECT_EQ(rc, 0);
}

TEST(JudgeClientParallel) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    test_hooks::state().compile_result = 0;
    test_hooks::state().compare_result = ACCEPT;
    TempDir tmp;
    write_basic_config(tmp.path, true, false);
    write_submission(tmp.path, "t3", 1001, false);
    prepare_case_files(tmp.path, 1001);
    prepare_run_dir(tmp.path, 1, true);
    std::string root = tmp.path.string();
    const char *argv[] = {
        "judge_client",
        "-solution_id", "1",
        "-runner_id", "1",
        "-dir", root.c_str(),
        "-judger_id", "t3",
        "-no-mysql",
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    int rc = judge_client_main(argc, const_cast<char **>(argv));
    EXPECT_EQ(rc, 0);
}

TEST(JudgeClientTestRun) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    test_hooks::state().compile_result = 0;
    test_hooks::state().wait4_status = SIGXCPU << 8;
    TempDir tmp;
    write_basic_config(tmp.path, false, false);
    write_submission(tmp.path, "t4", 0, true);
    prepare_run_dir(tmp.path, 1, false);
    std::string root = tmp.path.string();
    const char *argv[] = {
        "judge_client",
        "-solution_id", "1",
        "-runner_id", "1",
        "-dir", root.c_str(),
        "-judger_id", "t4",
        "-no-mysql",
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    expect_exit([&]() { judge_client_main(argc, const_cast<char **>(argv)); }, 0);
}

TEST(JudgeClientRuntimeInfoBranches) {
    test_hooks::reset();
    ScopedGlobalRuntimeGuard runtime_guard;
    test_hooks::state().compile_result = 0;
    test_hooks::state().compare_result = WRONG_ANSWER;
    TempDir tmp;
    write_basic_config(tmp.path, false, false);
    write_submission(tmp.path, "t5", 1001, false);
    prepare_case_files(tmp.path, 1001);
    prepare_run_dir(tmp.path, 1, false);
    std::string root = tmp.path.string();
    const char *argv1[] = {
        "judge_client",
        "-solution_id", "1",
        "-runner_id", "1",
        "-dir", root.c_str(),
        "-judger_id", "t5",
        "-no-mysql",
    };
    int argc1 = sizeof(argv1) / sizeof(argv1[0]);
    int rc = judge_client_main(argc1, const_cast<char **>(argv1));
    EXPECT_EQ(rc, 0);

    test_hooks::state().compare_result = RUNTIME_ERROR;
    write_submission(tmp.path, "t6", 1001, false);
    const char *argv2[] = {
        "judge_client",
        "-solution_id", "1",
        "-runner_id", "1",
        "-dir", root.c_str(),
        "-judger_id", "t6",
        "-no-mysql",
    };
    int argc2 = sizeof(argv2) / sizeof(argv2[0]);
    rc = judge_client_main(argc2, const_cast<char **>(argv2));
    EXPECT_EQ(rc, 0);
}



