#include "test_common.h"

TEST(JSONVectorReaderBasics) {
    test_hooks::reset();
    JSONVectorReader reader;
    std::string json = "{\"arr\":[\"a\",\"b\"],\"str\":\"x\",\"i\":3,"
                       "\"b\":true,\"d\":1.5,\"obj\":{\"k\":1}}";
    EXPECT_TRUE(reader.loadJSON(json));
    auto arr = reader.GetArray("arr");
    EXPECT_EQ(arr.size(), 2u);
    EXPECT_EQ(reader.GetString("str"), "x");
    EXPECT_EQ(reader.GetInt("i"), 3);
    EXPECT_EQ(reader.GetBool("b"), true);
    EXPECT_EQ(reader.GetDouble("d"), 1.5);
    EXPECT_EQ(reader.GetDouble("i"), 3);
    EXPECT_TRUE(reader.GetObject("obj") != nullptr);
    EXPECT_TRUE(reader.has("i"));
    EXPECT_EQ(reader.GetArray("missing").size(), 0u);
    EXPECT_EQ(reader.GetInt("missing"), -1);
    EXPECT_EQ(reader.GetString("missing"), "");
}

TEST(JSONVectorReaderLoadFile) {
    test_hooks::reset();
    TempDir tmp;
    auto file = tmp.path / "cfg.json";
    write_file(file, "{\"x\":1}");
    JSONVectorReader reader(file.string());
    EXPECT_EQ(reader.GetInt("x"), 1);
}

TEST(SubmissionInfoBasics) {
    SubmissionInfo info;
    std::string json = "{\"language\":0,\"user_id\":\"u\",\"problem_id\":1,"
                       "\"spj\":false,\"memory_limit\":64,\"time_limit\":1,"
                       "\"source\":\"code\",\"solution_id\":2}";
    info.readJSON(json);
    EXPECT_EQ(info.getLanguage(), 0);
    EXPECT_EQ(info.getProblemId(), 1);
    EXPECT_EQ(info.getSolutionId(), 2);
}

TEST(SubmissionInfoHelpers) {
    test_hooks::reset();
    TempDir tmp;
    std::strcpy(oj_home, tmp.path.string().c_str());
    std::string judger = "t5";
    write_file(tmp.path / "submission" / "t5.json",
               "{\"language\":0,\"user_id\":\"u\",\"problem_id\":1,"
               "\"spj\":false,\"memory_limit\":64,\"time_limit\":1,"
               "\"source\":\"code\",\"solution_id\":2}");
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    SubmissionInfo info;
    buildSubmissionInfo(info, judger);
    int pid = 0, lang = 0;
    char uid[128] = {};
    getSolutionInfoFromSubmissionInfo(info, pid, uid, lang);
    EXPECT_EQ(pid, 1);
    char code[CODESIZE] = {};
    getSolutionFromSubmissionInfo(info, code, tmp.path.string().c_str());
    EXPECT_TRUE(std::string(code).find("code") != std::string::npos);
    EXPECT_TRUE(std::filesystem::exists(tmp.path / "Main.txt"));
}

TEST(ConfigInfoReadFile) {
    TempDir tmp;
    auto f = tmp.path / "config.json";
    write_file(f, "{\"hostname\":\"h\",\"username\":\"u\",\"password\":\"p\","
                  "\"dbName\":\"d\",\"port\":1,\"java_time_bonus\":2,"
                  "\"java_memory_bonus\":3,\"sim_enable\":1,\"java_xms\":\"x\","
                  "\"java_xmx\":\"y\",\"http_judge\":\"0\",\"http_username\":\"h\","
                  "\"http_password\":\"p\",\"all_test_mode\":1,\"full_diff\":1,"
                  "\"share_memory_run\":0,\"use_max_time\":1,\"use_ptrace\":0}");
    ConfigInfo cfg;
    std::string config_path = f.string();
    cfg.readFromFile(config_path);
    EXPECT_EQ(cfg.getPort(), 1);
    EXPECT_EQ(cfg.getSimEnable(), 1);
    EXPECT_EQ(cfg.getJavaTimeBonus(), 2);
    EXPECT_EQ(cfg.getJavaMemoryBonus(), 3);
    EXPECT_EQ(cfg.getHostname(), "h");
    EXPECT_EQ(cfg.getUserName(), "u");
    EXPECT_EQ(cfg.getPassword(), "p");
}

TEST(CompilerConfigReaderBasics) {
    CompilerConfigReader reader;
    CompilerConfigReader reader2(std::string("missing.json"));
    reader.setLanguage(CPP11);
    EXPECT_EQ(reader.getLanguage(), CPP11);
    EXPECT_EQ(reader.getCompilerArgs().size(), 0u);
    EXPECT_EQ(reader.getCompilerArgs(CPP11).size(), 0u);
    reader.clear();
}

TEST(ConfigInfoSetters) {
    ConfigInfo cfg;
    std::string hostname = "h2";
    std::string username = "u2";
    std::string password = "p2";
    std::string db = "d2";
    std::string xms = "xms";
    std::string xmx = "xmx";
    std::string http = "http";
    std::string http_user = "hu";
    std::string http_pass = "hp";
    cfg.setHostname(hostname)
        .setUserName(username)
        .setPassword(password)
        .setDbName(db)
        .setPort(3307)
        .setJavaTimeBonus(9)
        .setJavaMemoryBonus(10)
        .setSimEnable(1)
        .setJavaXms(xms)
        .setJavaXmx(xmx)
        .setHttpJudge(http)
        .setHttpUserName(http_user)
        .setHttpPassword(http_pass)
        .setAllTestMode(1)
        .setFullDiff(1)
        .setShareMemoryRun(1)
        .setUseMaxTime(1)
        .setUsePtrace(1);
    EXPECT_EQ(cfg.getHostname(), hostname);
    EXPECT_EQ(cfg.getUserName(), username);
    EXPECT_EQ(cfg.getPassword(), password);
    EXPECT_EQ(cfg.getDbName(), db);
    EXPECT_EQ(cfg.getPort(), 3307);
    EXPECT_EQ(cfg.getJavaTimeBonus(), 9);
    EXPECT_EQ(cfg.getJavaMemoryBonus(), 10);
    EXPECT_EQ(cfg.getSimEnable(), 1);
    EXPECT_EQ(cfg.getJavaXms(), xms);
    EXPECT_EQ(cfg.getJavaXmx(), xmx);
    EXPECT_EQ(cfg.getHttpJudge(), http);
    EXPECT_EQ(cfg.getHttpUserName(), http_user);
    EXPECT_EQ(cfg.getHttpPassword(), http_pass);
    EXPECT_EQ(cfg.getAllTestMode(), 1);
    EXPECT_EQ(cfg.getFullDiff(), 1);
    EXPECT_EQ(cfg.getShareMemoryRun(), 1);
    EXPECT_EQ(cfg.getUseMaxTime(), 1);
    EXPECT_EQ(cfg.getUsePtrace(), 1);
}

TEST(SubmissionInfoSetters) {
    SubmissionInfo info;
    std::string source = "code";
    std::string custom = "input";
    std::string user = "u";
    info.setSource(source)
        .setCustomInput(custom)
        .setTimeLimit(2.5)
        .setMemoryLimit(128)
        .setSpecialJudge(true)
        .setProblemId(12)
        .setUserId(user)
        .setLanguage(3)
        .setSolutionId(99);
    EXPECT_EQ(info.getSource(), source);
    EXPECT_EQ(info.getCustomInput(), custom);
    EXPECT_EQ(info.getTimeLimit(), 2.5);
    EXPECT_EQ(info.getMemoryLimit(), 128);
    EXPECT_EQ(info.getSpecialJudge(), true);
    EXPECT_EQ(info.getProblemId(), 12);
    EXPECT_EQ(info.getUserId(), user);
    EXPECT_EQ(info.getLanguage(), 3);
    EXPECT_EQ(info.getSolutionId(), 99);
    std::string json = "{\"language\":1,\"user_id\":\"u\",\"problem_id\":2,"
                       "\"spj\":true,\"memory_limit\":64,\"time_limit\":1,"
                       "\"source\":\"code\",\"solution_id\":2,"
                       "\"test_run\":true,\"custom_input\":\"x\"}";
    info.readJSON(json);
    EXPECT_EQ(info.getCustomInput(), "x");
    EXPECT_EQ(info.getSpecialJudge(), true);
}

TEST(SubmissionInfoReadFileNoTestRun) {
    TempDir tmp;
    auto f = tmp.path / "submission.json";
    write_file(f, "{\"language\":2,\"user_id\":\"u\",\"problem_id\":3,"
                  "\"spj\":false,\"memory_limit\":64,\"time_limit\":1.5,"
                  "\"source\":\"code\",\"solution_id\":9}");
    SubmissionInfo info;
    std::string path = f.string();
    info.readFromFile(path);
    EXPECT_EQ(info.getLanguage(), 2);
    EXPECT_EQ(info.getUserId(), "u");
    EXPECT_EQ(info.getProblemId(), 3);
    EXPECT_EQ(info.getSpecialJudge(), false);
    EXPECT_EQ(info.getCustomInput(), "");
}

TEST(SubmissionInfoFromStdin) {
    test_hooks::reset();
    TempDir tmp;
    std::strcpy(oj_home, tmp.path.string().c_str());
    READ_FROM_STDIN = true;
    std::istringstream input(
        "{\"language\":0,\"user_id\":\"u\",\"problem_id\":1,"
        "\"spj\":false,\"memory_limit\":64,\"time_limit\":1,"
        "\"source\":\"code\",\"solution_id\":2}");
    auto *old_buf = std::cin.rdbuf(input.rdbuf());
    SubmissionInfo info;
    std::string judger = "stdin";
    buildSubmissionInfo(info, judger);
    std::cin.rdbuf(old_buf);
    READ_FROM_STDIN = false;
    EXPECT_EQ(info.getProblemId(), 1);
    EXPECT_EQ(info.getSolutionId(), 2);
}

TEST(JSONVectorReaderErrors) {
    JSONVectorReader reader;
    EXPECT_TRUE(!reader.loadJSON("{"));
    try {
        reader.GetString("x");
        fail("expected exception");
    } catch (const char *msg) {
        EXPECT_TRUE(std::string(msg).find("No JSON") != std::string::npos);
    }
    JSONVectorReader ok;
    EXPECT_TRUE(ok.loadJSON("{\"b\":true,\"n\":1,\"d\":1.25}"));
    EXPECT_TRUE(ok.GetBool("b"));
    EXPECT_EQ(ok.GetDouble("n"), 1);
    EXPECT_EQ(ok.GetDouble("d"), 1.25);
}
