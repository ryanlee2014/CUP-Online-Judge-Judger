#include "test_common.h"

TEST(ThreadPoolBasics) {
    ThreadPool pool(2);
    auto f1 = pool.enqueue([] { return 2; });
    auto f2 = pool.enqueue([](int a, int b) { return a + b; }, 3, 4);
    EXPECT_EQ(f1.get(), 2);
    EXPECT_EQ(f2.get(), 7);
}

TEST(JudgeLibUtils) {
    char buf[128] = "A=B";
    EXPECT_EQ(after_equal(buf), 2);
    char tbuf[128] = "  value  ";
    trim(tbuf);
    EXPECT_EQ(std::string(tbuf), "value");
    char keybuf[128] = "KEY=1";
    char out[128];
    EXPECT_TRUE(read_buf(keybuf, "KEY", out));
    int v = 0;
    read_int(keybuf, "KEY", v);
    EXPECT_EQ(v, 1);
    EXPECT_EQ(std::string(getFileNameFromPath("/a/b/c.txt")), "c.txt");
    EXPECT_EQ(isInFile("a.in"), 1);
    char esc[128];
    escape(esc, "a'b");
    EXPECT_TRUE(std::string(esc).find("\\'") != std::string::npos);
    EXPECT_EQ(detectArgType("-solution_id"), _SOLUTION_ID);
    EXPECT_EQ(detectArgType("DEBUG"), _DEBUG);
}

TEST(JudgeLibArgs) {
    EXPECT_EQ(detectArgType("-language"), _LANG_NAME);
    EXPECT_EQ(detectArgType("-no_record"), _NO_RECORD);
    EXPECT_EQ(detectArgType("-dir"), _DIR);
    EXPECT_EQ(detectArgType("-record"), _RECORD_CALL);
    EXPECT_EQ(detectArgType("-runner_id"), _RUNNER_ID);
    EXPECT_EQ(detectArgType("-admin"), _ADMIN);
    EXPECT_EQ(detectArgType("-no-sim"), _NO_SIM);
    EXPECT_EQ(detectArgType("-no-mysql"), _NO_MYSQL);
    EXPECT_EQ(detectArgType("-judger_id"), _JUDGER_ID);
    EXPECT_EQ(detectArgType("-stdin"), _STDIN);
    EXPECT_EQ(detectArgType("-unknown"), _ERROR);
    EXPECT_EQ(detectArgType("BAD"), _ERROR);
}

TEST(JudgeLibFiles) {
    TempDir tmp;
    auto f = tmp.path / "file.txt";
    write_file(f, "abc");
    EXPECT_EQ(get_file_size(f.string().c_str()), 3);
    auto err = tmp.path / "error.out";
    write_file(err, "err");
    EXPECT_EQ(getRuntimeInfoContents(err.string()), "err");
    EXPECT_EQ(getRuntimeInfoContents((tmp.path / "missing.out").string()), "");
    auto dir = tmp.path / "data";
    write_file(dir / "1.in", "");
    write_file(dir / "a.txt", "");
    auto list = getFileList(dir.string(), isInFile);
    EXPECT_EQ(list.size(), 1u);
}

TEST(JudgeLibExec) {
    test_hooks::reset();
    test_hooks::state().system_result = 0;
    EXPECT_EQ(execute_timeout_cmd(1, "true"), 0);
}

TEST(JudgeLibWorkdir) {
    test_hooks::reset();
    TempDir tmp;
    std::strcpy(oj_home, tmp.path.string().c_str());
    char work_dir[512];
    std::snprintf(work_dir, sizeof(work_dir), "%s/run1", oj_home);
    make_workdir(work_dir);
    mk_shm_workdir(work_dir);
    clean_workdir(work_dir);
    umount(work_dir);
}

TEST(JudgeLibPrepareFiles) {
    test_hooks::reset();
    TempDir tmp;
    std::strcpy(oj_home, tmp.path.string().c_str());
    std::string work_dir = tmp.path.string();
    auto data_dir = tmp.path / "data" / "1";
    write_file(data_dir / "1.in", "input");
    write_file(data_dir / "sample.dic", "dic");
    char infile[512], outfile[512], userfile[512];
    int pid = 1;
    prepare_files("1.in", 1, infile, pid,
                  work_dir.data(),
                  outfile, userfile, 2);
    EXPECT_TRUE(std::string(infile).find("/data/") != std::string::npos);
    EXPECT_EQ(read_file(tmp.path / "data.in"), "input");
    EXPECT_EQ(read_file(tmp.path / "sample.dic"), "dic");
    prepare_files_with_id("1.in", 1, infile, pid,
                          work_dir.data(),
                          outfile, userfile, 2, 3);
    EXPECT_EQ(read_file(tmp.path / "data3.in"), "input");
}

TEST(InitMysqlConfFromConfig) {
    TempDir tmp;
    auto config_dir = tmp.path / "etc";
    std::filesystem::create_directories(config_dir);
    auto f = config_dir / "config.json";
    write_file(f, "{\"hostname\":\"host\",\"username\":\"user\",\"password\":\"pass\","
                  "\"db_name\":\"db\",\"port\":3307,\"java_time_bonus\":7,"
                  "\"java_memory_bonus\":8,\"sim_enable\":1,\"java_xms\":\"-Xms16m\","
                  "\"java_xmx\":\"-Xmx128m\",\"full_diff\":1,\"judger_name\":\"j\","
                  "\"shm_run\":1,\"use_max_time\":1,\"use_ptrace\":0,"
                  "\"all_test_mode\":1,\"enable_parallel\":1}");
    JudgeContext ctx;
    ctx.env.oj_home = tmp.path.string();
    MysqlConfigValues cfg = read_mysql_config(f.string());
    apply_mysql_config(cfg, ctx);
    EXPECT_EQ(ctx.env.host_name, "host");
    EXPECT_EQ(ctx.env.user_name, "user");
    EXPECT_EQ(ctx.env.password, "pass");
    EXPECT_EQ(ctx.env.db_name, "db");
    EXPECT_EQ(ctx.env.database_port, 3307);
    EXPECT_EQ(ctx.config.java_time_bonus, 7);
    EXPECT_EQ(ctx.config.java_memory_bonus, 8);
    EXPECT_EQ(ctx.config.sim_enable, 1);
    EXPECT_EQ(ctx.config.full_diff, 1);
    EXPECT_EQ(ctx.config.share_memory_run, 1);
    EXPECT_EQ(ctx.config.use_max_time, 1);
    EXPECT_EQ(ctx.config.use_ptrace, 0);
    EXPECT_EQ(ctx.config.all_test_mode, 1);
    EXPECT_EQ(ctx.config.enable_parallel, 1);
}

TEST(ProcessAndLanguageHelpers) {
    test_hooks::reset();
    int rss = get_proc_status(getpid(), "VmRSS:");
    EXPECT_TRUE(rss >= 0);
    EXPECT_TRUE(isPython(PYTHON2));
    EXPECT_TRUE(!isPython(CPP11));
    setRunUser();
}

TEST(InitManagerBasics) {
    test_hooks::reset();
    languageNameReader.loadJSON("{\"0\":\"c11\"}");
    int call_counter[512] = {};
    InitManager::initSyscallLimits(0, call_counter, 0, 512);
    EXPECT_TRUE(call_counter[0] == HOJ_MAX_LIMIT);
}

TEST(InitManagerRecordCallDebug) {
    test_hooks::reset();
    TempDir tmp;
    std::strcpy(oj_home, tmp.path.string().c_str());
    DEBUG = 1;
    int call_counter[16];
    for (int &v : call_counter) {
        v = 1;
    }
    InitManager::initSyscallLimits(0, call_counter, 1, 16);
    for (int v : call_counter) {
        EXPECT_EQ(v, 0);
    }
    DEBUG = 0;
}

TEST(SeccompStubCoverage) {
    scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_ALLOW);
    EXPECT_TRUE(ctx != nullptr);
    EXPECT_EQ(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, 0, 0), 0);
    EXPECT_EQ(seccomp_load(ctx), 0);
}

TEST(SeccompHelperBasics) {
    if (!run_seccomp()) {
        return;
    }
    char buf[32] = {};
    write_uint(buf, 0);
    EXPECT_EQ(std::string(buf), "0");
    write_uint(buf, 123);
    EXPECT_EQ(std::string(buf), "123");
    EXPECT_EQ(install_helper(), 0);
    siginfo_t info{};
    info.si_code = 0;
    helper(0, &info, nullptr);
#ifdef SYS_SECCOMP
    info.si_code = SYS_SECCOMP;
#else
    info.si_code = 1;
#endif
    helper(0, &info, nullptr);
    ucontext_t ctx{};
#if defined(__x86_64__)
    ctx.uc_mcontext.gregs[REG_RAX] = 1;
#elif defined(__i386__)
    ctx.uc_mcontext.gregs[REG_EAX] = 1;
#endif
    expect_exit([&]() { helper(0, &info, &ctx); }, 1);
}

TEST(BonusLimitBasics) {
    BonusLimitTester tester;
    EXPECT_EQ(tester.time(1.0, 2.0), 4.0);
    EXPECT_EQ(tester.memory(64, 10), 74);
}

TEST(SeccompHelperCoverage) {
    char buf[32] = {};
    write_uint(buf, 0);
    EXPECT_EQ(std::string(buf), "0");
    write_uint(buf, 42);
    EXPECT_EQ(std::string(buf), "42");
    siginfo_t info{};
    ucontext_t ctx{};
    info.si_code = 0;
    helper(0, &info, &ctx);
    info.si_code = SYS_SECCOMP;
    helper(0, &info, nullptr);
#if defined(__x86_64__)
    ctx.uc_mcontext.gregs[REG_RAX] = 1;
#elif defined(__i386__)
    ctx.uc_mcontext.gregs[REG_EAX] = 1;
#endif
    expect_exit([&]() { helper(0, &info, &ctx); }, 1);
#if defined(__x86_64__)
    ctx.uc_mcontext.gregs[REG_RAX] = 100000;
#elif defined(__i386__)
    ctx.uc_mcontext.gregs[REG_EAX] = 100000;
#endif
    expect_exit([&]() { helper(0, &info, &ctx); }, 1);
    EXPECT_EQ(install_helper(), 0);
}

TEST(SeccompHelperBuildFilter) {
    int syscalls[] = {1, 2, 0};
    char *args[] = {const_cast<char *>("prog"), nullptr};
    EXPECT_EQ(build_seccomp_filter(syscalls, true, args), 0);
    EXPECT_EQ(build_seccomp_filter(syscalls, false, nullptr), 0);
}

TEST(TestHooksExtraCoverage) {
    test_hooks::reset();
    TempDir tmp;
    auto *adapter = getAdapter();
    test_hooks::state().mysql_custom_input = "input";
    adapter->getCustomInput(1, const_cast<char *>(tmp.path.string().c_str()));
    EXPECT_TRUE(std::filesystem::exists(tmp.path / "data.in"));

    test_hooks::state().mysql_source = "code";
    char code[128] = {};
    adapter->getSolution(1, const_cast<char *>(tmp.path.string().c_str()), 0, code, "txt", 0);
    EXPECT_TRUE(std::string(code).find("code") != std::string::npos);
    EXPECT_TRUE(adapter->isConnected());

    auto destroy_fn = reinterpret_cast<void (*)(void *)>(test_dlsym(nullptr, "destroyInstance"));
    EXPECT_TRUE(destroy_fn != nullptr);
    destroy_fn(nullptr);
    EXPECT_TRUE(test_dlsym(nullptr, "unknown_symbol") == nullptr);

    test_hooks::state().pipe_fail = true;
    test_hooks::state().pipe_errno = EPIPE;
    int fds[2] = {};
    EXPECT_EQ(test_pipe(fds), -1);

    void *mem = std::malloc(32);
    MYSQL *mysql = reinterpret_cast<MYSQL *>(mem);
    mysql_init(mysql);
    std::free(mem);
    EXPECT_TRUE(mysql_store_result(nullptr) == nullptr);

    test_hooks::state().pipe_keep_fd = dup(fileno(stdout));
    test_hooks::push_mysql_result({{"1"}});
    test_hooks::reset();
}

TEST(JudgeLibExtraFunctions) {
    test_hooks::reset();
    TempDir tmp;
    std::string root = tmp.path.string();
    std::strcpy(oj_home, root.c_str());
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(tmp.path);
    char line[] = "abc\r\n";
    delnextline(line);
    EXPECT_EQ(std::string(line), "abc");
    FILE *pf = read_cmd_output("echo hi");
    char buf[16] = {};
    if (pf) {
        fgets(buf, sizeof(buf), pf);
        pclose(pf);
    }
    EXPECT_TRUE(std::string(buf).find("hi") != std::string::npos);
    write_log(oj_home, "log %d", 1);
    std::string ip = "127.0.0.1";
    EXPECT_TRUE(initWebSocketConnection(ip, 1));
    EXPECT_TRUE(initWebSocketConnection(std::string("127.0.0.1"), 2));
    std::string uuid = "x";
    removeSubmissionInfo(uuid);
    expect_exit([&]() { getFileList((tmp.path / "missing").string(), isInFile); }, -1);
    write_file(tmp.path / "data" / "1.in", "");
    auto list = getFileList((tmp.path / "data").string());
    EXPECT_TRUE(list.size() >= 1u);
    DEBUG = 1;
    clean_workdir(const_cast<char *>(root.c_str()));
    DEBUG = 0;
    clean_workdir(const_cast<char *>(root.c_str()));
    std::string big(5000, 'a');
    write_file(tmp.path / "big.txt", big);
    auto runtime_info = getRuntimeInfoContents((tmp.path / "big.txt").string());
    EXPECT_TRUE(runtime_info.size() > 4096);
    EXPECT_TRUE(runtime_info.size() <= 5120);
    print_runtimeerror("oops", root.c_str());
    EXPECT_TRUE(std::filesystem::exists(tmp.path / "error.out"));
    std::filesystem::current_path(old_cwd);
}

TEST(JudgeLibDynamicInstances) {
    test_hooks::reset();
    languageNameReader.loadJSON("{\"1\":\"c11\"}");
    std::unique_ptr<Language> lang(getLanguageModel(1));
    EXPECT_TRUE(lang != nullptr);
    auto *adapter = getAdapter();
    EXPECT_TRUE(adapter != nullptr);
    std::unique_ptr<Compare::Compare> compare(getCompareModel());
    EXPECT_TRUE(compare != nullptr);
}

TEST(JudgeLibDynamicInstanceCache) {
    test_hooks::reset();
    languageNameReader.loadJSON("{\"1\":\"c11\"}");
    auto &st = test_hooks::state();
    int before_dlopen = st.dlopen_calls;
    int before_dlsym = st.dlsym_calls;
    std::unique_ptr<Language> lang1(getLanguageModel(1));
    std::unique_ptr<Language> lang2(getLanguageModel(1));
    EXPECT_TRUE(lang1 != nullptr);
    EXPECT_TRUE(lang2 != nullptr);
    int dlopen_delta = st.dlopen_calls - before_dlopen;
    int dlsym_delta = st.dlsym_calls - before_dlsym;
    EXPECT_TRUE(dlopen_delta == 0 || dlopen_delta == 1);
    EXPECT_TRUE(dlsym_delta == 0 || dlsym_delta == 1);
}

TEST(JudgeLibGetSimBranches) {
    test_hooks::reset();
    TempDir tmp;
    std::string root = tmp.path.string();
    std::strcpy(oj_home, root.c_str());
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(tmp.path);
    languageNameReader.loadJSON("{\"1\":\"c11\",\"2\":\"cpp11\"}");
    no_sim = true;
    int sim_s_id = 0;
    EXPECT_EQ(get_sim(1, 1, 1, sim_s_id), 0);
    no_sim = false;
    admin = false;
    DEBUG = 1;
    test_hooks::state().system_result = 0;
    write_file(tmp.path / "Main.c", "int main(){}");
    EXPECT_EQ(get_sim(1, 1, 1, sim_s_id), 0);
    write_file(tmp.path / "Main.cc", "int main(){}");
    EXPECT_EQ(get_sim(2, 2, 2, sim_s_id), 0);
    test_hooks::state().system_result = 1;
    write_file(tmp.path / "sim", "1 2");
    EXPECT_EQ(get_sim(3, 1, 1, sim_s_id), 1);
    EXPECT_EQ(sim_s_id, 2);
    DEBUG = 0;
    admin = false;
    no_sim = false;
    std::filesystem::current_path(old_cwd);
}

TEST(JudgeSolutionTimeAndMemory) {
    test_hooks::reset();
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    TempDir tmp;
    std::strcpy(oj_home, tmp.path.string().c_str());
    std::string work = tmp.path.string();
    std::string global = work + "/";
    int ac = ACCEPT;
    double used = 2000.0;
    double tl = 1.0;
    int top = 128 * STD_MB;
    int pe = 0;
    char infile[] = "in";
    char outfile[] = "out";
    char userfile[] = "user";
    char code[] = "code";
    ALL_TEST_MODE = 0;
    use_max_time = 1;
    JudgeEnv env = capture_env();
    bool debug_enabled = false;
    auto res = judge_solution(ac, used, tl, 0, 1, infile, outfile, userfile, code,
                              pe, 0, work.data(), top, 64, 1, 1, global, make_config_snapshot(),
                              env, debug_enabled);
    EXPECT_EQ(res.ACflg, MEMORY_LIMIT_EXCEEDED);
    ALL_TEST_MODE = 1;
}

TEST(JudgeSolutionCompareResults) {
    test_hooks::reset();
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    TempDir tmp;
    std::strcpy(oj_home, tmp.path.string().c_str());
    std::string work = tmp.path.string();
    std::string global = work + "/";
    int pe = 0;
    char infile[] = "in";
    char outfile[] = "out";
    char userfile[] = "user";
    char code[] = "code";

    DEBUG = 1;
    JudgeEnv env = capture_env();
    bool debug_enabled = true;
    test_hooks::state().compare_result = WRONG_ANSWER;
    int ac = ACCEPT;
    double used = 0.0;
    double tl = 1.0;
    int top = 0;
    auto res = judge_solution(ac, used, tl, 0, 1, infile, outfile, userfile, code,
                              pe, 0, work.data(), top, 64, 1, 1, global, make_config_snapshot(),
                              env, debug_enabled);
    EXPECT_EQ(res.ACflg, WRONG_ANSWER);

    test_hooks::state().compare_result = PRESENTATION_ERROR;
    ac = ACCEPT;
    used = 0.0;
    top = 0;
    pe = 0;
    res = judge_solution(ac, used, tl, 0, 1, infile, outfile, userfile, code,
                         pe, 0, work.data(), top, 64, 1, 1, global, make_config_snapshot(),
                         env, debug_enabled);
    EXPECT_EQ(res.ACflg, PRESENTATION_ERROR);
    EXPECT_EQ(pe, PRESENTATION_ERROR);
    DEBUG = 0;
}

TEST(JudgeSolutionSpecialJudge) {
    test_hooks::reset();
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    TempDir tmp;
    std::strcpy(oj_home, tmp.path.string().c_str());
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(tmp.path);
    write_file(std::filesystem::path("user"), "1\n");
    std::string work = tmp.path.string();
    std::string global = work + "/";
    int ac = ACCEPT;
    double used = 0.0;
    double tl = 1.0;
    int top = 0;
    int pe = 0;
    char infile[] = "in";
    char outfile[] = "out";
    char userfile[] = "user";
    char code[] = "code";
    JudgeEnv env = capture_env();
    bool debug_enabled = false;
    test_hooks::state().fork_results.push_back(1);
    test_hooks::state().pipe_seed = "0";
    int stdout_fd = dup(fileno(stdout));
    auto res = judge_solution(ac, used, tl, 1, 1, infile, outfile, userfile, code,
                              pe, 0, work.data(), top, 64, 1, 1, global, make_config_snapshot(),
                              env, debug_enabled);
    EXPECT_EQ(res.ACflg, ACCEPT);
    if (stdout_fd >= 0) {
        fflush(stdout);
        dup2(stdout_fd, fileno(stdout));
        close(stdout_fd);
    }
    std::filesystem::current_path(old_cwd);
}
