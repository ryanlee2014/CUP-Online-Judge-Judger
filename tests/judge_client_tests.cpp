#include "test_common.h"

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
    const char *argv[] = {"wsjudged", "1", "2", "/tmp", "DEBUG"};
    EXPECT_EQ(wsjudged_main(5, argv), 0);
    EXPECT_EQ(test_hooks::state().last_exec_path, "/usr/bin/judge_client");
}

TEST(JudgeClientInitParameters) {
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
    DEBUG = 0;
    NO_RECORD = 0;
    record_call = 0;
    admin = false;
    no_sim = false;
    MYSQL_MODE = true;
    READ_FROM_STDIN = false;
}

TEST(JudgeClientInitParametersOldPathRecordCall) {
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
    record_call = 0;
    const char *argv2[] = {"judge_client", "1", "2"};
    int argc2 = sizeof(argv2) / sizeof(argv2[0]);
    init_parameters(argc2, const_cast<char **>(argv2), sid, rid, jid);
    EXPECT_EQ(std::string(oj_home), "/home/judge");
    DEBUG = 0;
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
    TempDir tmp;
    write_basic_config(tmp.path, false, false);
    std::strcpy(oj_home, tmp.path.string().c_str());
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    auto work_dir = tmp.path / "run1";
    std::filesystem::create_directories(work_dir);
    auto work_str = work_dir.string();
    DEBUG = 1;
    test_hooks::state().fork_results.push_back(0);
    JudgeEnv env = capture_env();
    JudgeConfigSnapshot cfg = make_config_snapshot();
    expect_exit([&]() { compile(0, work_str.data(), env, cfg, DEBUG != 0); }, 0);

    test_hooks::reset();
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    std::strcpy(oj_home, tmp.path.string().c_str());
    test_hooks::state().compile_result = 0;
    test_hooks::state().fork_results.push_back(123);
    env = capture_env();
    cfg = make_config_snapshot();
    EXPECT_EQ(compile(0, work_str.data(), env, cfg, DEBUG != 0), 0);
    DEBUG = 0;
}

TEST(JudgeClientCompileChildParent) {
    test_hooks::reset();
    TempDir tmp;
    write_basic_config(tmp.path, false, false);
    std::strcpy(oj_home, tmp.path.string().c_str());
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    auto work_dir = tmp.path / "run1";
    std::filesystem::create_directories(work_dir);
    auto work_str = work_dir.string();
    test_hooks::state().fork_results.push_back(0);
    JudgeEnv env = capture_env();
    JudgeConfigSnapshot cfg = make_config_snapshot();
    expect_exit([&]() { compile(0, work_str.data(), env, cfg, DEBUG != 0); }, 0);
    test_hooks::reset();
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    std::strcpy(oj_home, tmp.path.string().c_str());
    test_hooks::state().compile_result = 0;
    env = capture_env();
    cfg = make_config_snapshot();
    EXPECT_EQ(compile(0, work_str.data(), env, cfg, DEBUG != 0), 0);
}

TEST(JudgeClientRunSolutionPaths) {
    test_hooks::reset();
    TempDir tmp;
    auto old_cwd = std::filesystem::current_path();
    std::strcpy(oj_home, tmp.path.string().c_str());
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    write_file(tmp.path / "data.in", "1");
    write_file(tmp.path / "data1.in", "1");
    int lang = 0;
    double tl = 1.0;
    double used = 0.0;
    int mem = 64;
    ALL_TEST_MODE = 1;
    use_ptrace = 0;
    auto work = tmp.path.string();
    int stdout_fd = dup(fileno(stdout));
    int stderr_fd = dup(fileno(stderr));
    expect_exit([&]() { run_solution(lang, work.data(), tl, used, mem, make_config_snapshot()); }, 0);
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
    ALL_TEST_MODE = 0;
    use_ptrace = 1;
    stdout_fd = dup(fileno(stdout));
    stderr_fd = dup(fileno(stderr));
    expect_exit([&]() { run_solution_parallel(lang, work.data(), tl, used, mem, 1, make_config_snapshot()); }, 0);
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
    ALL_TEST_MODE = 1;
    use_ptrace = 1;
}

TEST(JudgeClientRunSolutionNonAllTest) {
    test_hooks::reset();
    TempDir tmp;
    auto old_cwd = std::filesystem::current_path();
    std::strcpy(oj_home, tmp.path.string().c_str());
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    write_file(tmp.path / "data.in", "1");
    int lang = 0;
    double tl = 1.0;
    double used = 500.0;
    int mem = 64;
    ALL_TEST_MODE = 0;
    use_ptrace = 0;
    auto work = tmp.path.string();
    int stdout_fd = dup(fileno(stdout));
    int stderr_fd = dup(fileno(stderr));
    expect_exit([&]() { run_solution(lang, work.data(), tl, used, mem, make_config_snapshot()); }, 0);
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
    ALL_TEST_MODE = 1;
}

TEST(JudgeClientRunSolutionPtrace) {
    test_hooks::reset();
    TempDir tmp;
    auto old_cwd = std::filesystem::current_path();
    std::strcpy(oj_home, tmp.path.string().c_str());
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    write_file(tmp.path / "data.in", "1");
    int lang = 0;
    double tl = 1.0;
    double used = 0.0;
    int mem = 64;
    ALL_TEST_MODE = 1;
    use_ptrace = 1;
    auto work = tmp.path.string();
    int stdout_fd = dup(fileno(stdout));
    int stderr_fd = dup(fileno(stderr));
    expect_exit([&]() { run_solution(lang, work.data(), tl, used, mem, make_config_snapshot()); }, 0);
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
    use_ptrace = 0;
}

TEST(JudgeClientWatchSolutionBranches) {
    test_hooks::reset();
    TempDir tmp;
    std::string root = tmp.path.string();
    std::strcpy(oj_home, root.c_str());
    JudgeEnv env = capture_env();
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
    bool record_syscall = record_call != 0;
    bool debug_enabled = DEBUG != 0;

    int ac = ACCEPT;
    int top = STD_MB * 2;
    test_hooks::state().wait4_status = 0;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), make_config_snapshot(),
                   env, record_syscall, debug_enabled);
    EXPECT_EQ(ac, MEMORY_LIMIT_EXCEEDED);

    ac = ACCEPT;
    top = 0;
    write_file(tmp.path / "error.out", "Killed");
    write_file(std::filesystem::path("error.out"), "Killed");
    test_hooks::state().wait4_status = 1;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, 64,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), make_config_snapshot(),
                   env, record_syscall, debug_enabled);
    EXPECT_TRUE(ac == ACCEPT || ac == RUNTIME_ERROR);

    ac = ACCEPT;
    top = 0;
    write_file(user_path, std::string(3000, 'a'));
    write_file(out_path, "1");
    write_file(tmp.path / "error.out", "");
    test_hooks::state().wait4_status = 1;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, 64,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), make_config_snapshot(),
                   env, record_syscall, debug_enabled);
    EXPECT_EQ(ac, OUTPUT_LIMIT_EXCEEDED);

    ac = ACCEPT;
    top = 0;
    write_file(user_path, "1");
    test_hooks::state().wait4_status = (SIGXCPU << 8) | 1;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, 64,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), make_config_snapshot(),
                   env, record_syscall, debug_enabled);
    EXPECT_TRUE(ac != ACCEPT);

    ac = ACCEPT;
    top = 0;
    test_hooks::state().wait4_status = SIGXCPU;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, 64,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), make_config_snapshot(),
                   env, record_syscall, debug_enabled);
    EXPECT_TRUE(ac != ACCEPT);
    std::filesystem::current_path(old_cwd);
}

TEST(JudgeClientWatchSolutionKilledDebug) {
    test_hooks::reset();
    TempDir tmp;
    std::string root = tmp.path.string();
    std::strcpy(oj_home, root.c_str());
    JudgeEnv env = capture_env();
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
    bool record_syscall = record_call != 0;
    bool debug_enabled = DEBUG != 0;
    test_hooks::state().wait4_status = 1;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), make_config_snapshot(),
                   env, record_syscall, debug_enabled);
    DEBUG = 0;
    ALL_TEST_MODE = 1;
    std::filesystem::current_path(old_cwd);
}

TEST(JudgeClientWatchSolutionErrorAndExitStatus) {
    test_hooks::reset();
    TempDir tmp;
    std::string root = tmp.path.string();
    std::strcpy(oj_home, root.c_str());
    JudgeEnv env = capture_env();
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
    bool record_syscall = record_call != 0;
    bool debug_enabled = DEBUG != 0;
    test_hooks::state().wait4_status = SIGKILL;
    watch_solution(1, infile, ac, 1, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), make_config_snapshot(),
                   env, record_syscall, debug_enabled);
    EXPECT_TRUE(read_file(tmp.path / "error.out").find("Runtime Error:") != std::string::npos);

    write_file(tmp.path / "error.out", "");
    ac = ACCEPT;
    top = 0;
    test_hooks::state().wait4_status = SIGALRM;
    watch_solution(1, infile, ac, 1, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), make_config_snapshot(),
                   env, record_syscall, debug_enabled);
    EXPECT_EQ(ac, TIME_LIMIT_EXCEEDED);

    write_file(tmp.path / "error.out", "");
    ac = ACCEPT;
    top = 0;
    test_hooks::state().wait4_status = SIGXFSZ;
    watch_solution(1, infile, ac, 1, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), make_config_snapshot(),
                   env, record_syscall, debug_enabled);
    EXPECT_EQ(ac, OUTPUT_LIMIT_EXCEEDED);

    write_file(tmp.path / "error.out", "");
    ac = ACCEPT;
    top = 0;
    test_hooks::state().wait4_status = SIGILL;
    watch_solution(1, infile, ac, 1, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), make_config_snapshot(),
                   env, record_syscall, debug_enabled);
    EXPECT_EQ(ac, RUNTIME_ERROR);
    std::filesystem::current_path(old_cwd);
}

TEST(JudgeClientWatchSolutionExitcodeDebug) {
    test_hooks::reset();
    TempDir tmp;
    std::string root = tmp.path.string();
    std::strcpy(oj_home, root.c_str());
    JudgeEnv env = capture_env();
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
    bool record_syscall = record_call != 0;
    bool debug_enabled = DEBUG != 0;
    test_hooks::state().wait4_status = (SIGXFSZ << 8) | 1;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), make_config_snapshot(),
                   env, record_syscall, debug_enabled);
    EXPECT_EQ(ac, OUTPUT_LIMIT_EXCEEDED);
    DEBUG = 0;
    use_ptrace = 0;
    std::filesystem::current_path(old_cwd);
}

TEST(JudgeClientWatchSolutionSignalDebug) {
    test_hooks::reset();
    TempDir tmp;
    std::string root = tmp.path.string();
    std::strcpy(oj_home, root.c_str());
    JudgeEnv env = capture_env();
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
    bool record_syscall = record_call != 0;
    bool debug_enabled = DEBUG != 0;
    test_hooks::state().wait4_status = SIGXFSZ;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), make_config_snapshot(),
                   env, record_syscall, debug_enabled);
    EXPECT_EQ(ac, OUTPUT_LIMIT_EXCEEDED);
    DEBUG = 0;
    std::filesystem::current_path(old_cwd);
}

TEST(JudgeClientWatchSolutionMemoryLimitDebug) {
    test_hooks::reset();
    TempDir tmp;
    std::string root = tmp.path.string();
    std::strcpy(oj_home, root.c_str());
    JudgeEnv env = capture_env();
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
    bool record_syscall = record_call != 0;
    bool debug_enabled = DEBUG != 0;
    test_hooks::state().wait4_status = 1;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), make_config_snapshot(),
                   env, record_syscall, debug_enabled);
    DEBUG = 0;
    use_ptrace = 0;
    std::filesystem::current_path(old_cwd);
}

TEST(JudgeClientWatchSolutionRuntimeErrorBranch) {
    test_hooks::reset();
    TempDir tmp;
    std::string root = tmp.path.string();
    std::strcpy(oj_home, root.c_str());
    JudgeEnv env = capture_env();
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
    bool record_syscall = record_call != 0;
    bool debug_enabled = DEBUG != 0;
    test_hooks::state().wait4_status = 1;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), make_config_snapshot(),
                   env, record_syscall, debug_enabled);
    EXPECT_EQ(ac, RUNTIME_ERROR);
    use_ptrace = 0;
    ALL_TEST_MODE = 1;
    std::filesystem::current_path(old_cwd);
}

TEST(JudgeClientWatchSolutionOutputLimitPtrace) {
    test_hooks::reset();
    TempDir tmp;
    std::string root = tmp.path.string();
    std::strcpy(oj_home, root.c_str());
    JudgeEnv env = capture_env();
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
    bool record_syscall = record_call != 0;
    bool debug_enabled = DEBUG != 0;
    test_hooks::state().wait4_status = 1;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), make_config_snapshot(),
                   env, record_syscall, debug_enabled);
    EXPECT_EQ(ac, OUTPUT_LIMIT_EXCEEDED);
    use_ptrace = 0;
    std::filesystem::current_path(old_cwd);
}

TEST(JudgeClientWatchSolutionPtraceBranches) {
    test_hooks::reset();
    TempDir tmp;
    std::string root = tmp.path.string();
    std::strcpy(oj_home, root.c_str());
    JudgeEnv env = capture_env();
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
    bool record_syscall = record_call != 0;
    bool debug_enabled = DEBUG != 0;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), make_config_snapshot(),
                   env, record_syscall, debug_enabled);
    EXPECT_EQ(call_counter[12], 1);

    std::fill(call_counter, call_counter + call_array_size, 0);
    record_call = 0;
    test_hooks::state().wait4_statuses.push_back(0x7f);
    test_hooks::state().wait4_statuses.push_back(0);
    ac = ACCEPT;
    record_syscall = record_call != 0;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()), make_config_snapshot(),
                   env, record_syscall, debug_enabled);
    EXPECT_EQ(ac, RUNTIME_ERROR);
    use_ptrace = 0;
    record_call = 0;
    std::filesystem::current_path(old_cwd);
}

TEST(JudgeClientRunJudgeTaskBranches) {
    test_hooks::reset();
    TempDir tmp;
    std::string root = tmp.path.string();
    std::strcpy(oj_home, root.c_str());
    JudgeEnv env = capture_env();
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
    TempDir tmp;
    std::string root = tmp.path.string();
    std::strcpy(oj_home, root.c_str());
    JudgeEnv env = capture_env();
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
    reset_globals_for_test();
    TempDir tmp;
    std::string root = tmp.path.string();
    std::strcpy(oj_home, root.c_str());
    JudgeEnv env = capture_env();
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

TEST(JudgeClientWatchSolutionWithFileIdDebug) {
    test_hooks::reset();
    TempDir tmp;
    std::string root = tmp.path.string();
    std::strcpy(oj_home, root.c_str());
    JudgeEnv env = capture_env();
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
    bool record_syscall = record_call != 0;
    bool debug_enabled = DEBUG != 0;
    test_hooks::state().wait4_status = 1;
    watch_solution_with_file_id(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                                const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                                used, tl, p_id, pe, const_cast<char *>(root.c_str()), 1,
                                call_counter_local, make_config_snapshot(), env,
                                record_syscall, debug_enabled);
    DEBUG = 0;
    ALL_TEST_MODE = 1;
    std::filesystem::current_path(old_cwd);
}

TEST(JudgeClientWatchSolutionWithFileIdPtraceBranch) {
    test_hooks::reset();
    TempDir tmp;
    std::string root = tmp.path.string();
    std::strcpy(oj_home, root.c_str());
    JudgeEnv env = capture_env();
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
    bool record_syscall = record_call != 0;
    bool debug_enabled = DEBUG != 0;
    test_hooks::state().ptrace_syscall = 12;
    test_hooks::state().wait4_statuses.push_back(0x7f);
    test_hooks::state().wait4_statuses.push_back(0);
    watch_solution_with_file_id(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                                const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                                used, tl, p_id, pe, const_cast<char *>(root.c_str()), 1,
                                call_counter_local, make_config_snapshot(), env,
                                record_syscall, debug_enabled);
    EXPECT_EQ(call_counter_local[12], 1);
    use_ptrace = 0;
    record_call = 0;
    std::filesystem::current_path(old_cwd);
}

TEST(JudgeClientMySQLSim) {
    test_hooks::reset();
    reset_globals_for_test();
    TempDir tmp;
    std::string root = tmp.path.string();
    std::strcpy(oj_home, root.c_str());
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

TEST(WSJudgedBasics) {
    test_hooks::reset();
    const char *argv1[] = {"wsjudged"};
    EXPECT_EQ(wsjudged_main(1, argv1), 1);
    const char *argv2[] = {"wsjudged", "1", "2", "/tmp"};
    EXPECT_EQ(wsjudged_main(4, argv2), 0);
}

TEST(WSJudgedDebugArg) {
    test_hooks::reset();
    const char *argv[] = {"wsjudged", "1", "2", "/tmp", "debug"};
    EXPECT_EQ(wsjudged_main(5, argv), 0);
    EXPECT_EQ(test_hooks::state().last_exec_path, "/usr/bin/judge_client");
}
