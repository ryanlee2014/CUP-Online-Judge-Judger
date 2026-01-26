#include "test_hooks.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <chrono>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <ucontext.h>
#include <unistd.h>
#include <vector>

#include "../external/compare/CompareImpl.h"
#include "../external/mysql/MySQLAutoPointer.h"
#include "../external/mysql/MySQLSubmissionInfoManager.h"
#include "../header/static_var.h"
#include "../library/judge_lib.h"
#include "../manager/syscall/InitManager.h"
#include "../model/base/Bundle.h"
#include "../model/base/JSONVectorReader.h"
#include "../model/base/Pack.h"
#include "../model/base/ThreadPool.h"
#include "../model/config/ConfigInfo.h"
#include "../model/judge/language/Bash.h"
#include "../model/judge/language/C11.h"
#include "../model/judge/language/C99.h"
#include "../model/judge/language/Clang.h"
#include "../model/judge/language/Clang11.h"
#include "../model/judge/language/Clangpp.h"
#include "../model/judge/language/Clangpp17.h"
#include "../model/judge/language/Cpp11.h"
#include "../model/judge/language/Cpp17.h"
#include "../model/judge/language/Cpp20.h"
#include "../model/judge/language/Cpp98.h"
#include "../model/judge/language/Csharp.h"
#include "../model/judge/language/FreeBasic.h"
#include "../model/judge/language/Go.h"
#include "../model/judge/language/Java.h"
#include "../model/judge/language/Java6.h"
#include "../model/judge/language/Java7.h"
#include "../model/judge/language/Java8.h"
#include "../model/judge/language/JavaScript.h"
#include "../model/judge/language/Kotlin.h"
#include "../model/judge/language/KotlinNative.h"
#include "../model/judge/language/Lua.h"
#include "../model/judge/language/Objc.h"
#include "../model/judge/language/Pascal.h"
#include "../model/judge/language/Perl.h"
#include "../model/judge/language/Php.h"
#include "../model/judge/language/PyPy.h"
#include "../model/judge/language/PyPy3.h"
#include "../model/judge/language/Python2.h"
#include "../model/judge/language/Python3.h"
#include "../model/judge/language/Ruby.h"
#include "../model/judge/language/Schema.h"
#include "../model/judge/policy/SpecialJudge.h"
#include "../model/judge/language/common/seccomp_helper.h"
#include "../model/wrapper/CompilerConfigReader.h"
#include "../model/submission/SubmissionInfo.h"
#include "../model/websocket/WebSocketSender.h"

#ifndef SYS_SECCOMP
#define SYS_SECCOMP 1
#endif

extern "C" void __gcov_flush(void) __attribute__((weak));

int judge_client_main(int argc, char **argv);
int wsjudged_main(int argc, const char **argv);
bool check_valid_presentation_error(const char *ansfile, const char *userfile);
void find_next_nonspace(int &c1, int &c2, FILE *&f1, FILE *&f2, int &ret, int DEBUG);
int compile(int lang, char *work_dir);
FILE *read_cmd_output(const char *fmt, ...);
void init_mysql_conf();
void run_solution_parallel(int &lang, char *work_dir, double &time_lmt, double &usedtime,
                           int &mem_lmt, int fileId);
void run_solution(int &lang, char *work_dir, double &time_lmt, double &usedtime,
                  int &mem_lmt);
void watch_solution(pid_t pidApp, char *infile, int &ACflg, int isspj,
                    char *userfile, char *outfile, int solution_id, int lang,
                    int &topmemory, int mem_lmt, double &usedtime, double time_lmt, int &p_id,
                    int &PEflg, char *work_dir);
void watch_solution_with_file_id(pid_t pidApp, char *infile, int &ACflg, int isspj,
                                 char *userfile, char *outfile, int solution_id, int lang,
                                 int &topmemory, int mem_lmt, double &usedtime, double time_lmt, int &p_id,
                                 int &PEflg, char *work_dir, int file_id, int *call_counter);
JudgeResult runJudgeTask(int runner_id, int language, char *work_dir,
                         std::pair<std::string, int> &infilePair, int ACflg, int SPECIAL_JUDGE,
                         int solution_id, double timeLimit, double usedtime, int memoryLimit,
                         int problemId, char *usercode, int num_of_test, std::string &global_work_dir);
JudgeSeriesResult runParallelJudge(int runner_id, int language, char *work_dir, char *usercode,
                                   int timeLimit, int usedtime, int memoryLimit,
                                   std::vector<std::pair<std::string, int>> &inFileList,
                                   int &ACflg, int SPECIAL_JUDGE, std::string &global_work_dir,
                                   SubmissionInfo &submissionInfo);
void init_parameters(int argc, char **argv, int &solution_id,
                     int &runner_id, std::string &judgerId);
void print_call_array();
extern int call_counter[];
extern int choose;
JudgeResult judge_solution(int &ACflg, double &usedtime, double time_lmt, int isspj,
                           int p_id, char *infile, char *outfile, char *userfile, char *usercode, int &PEflg,
                           int lang, char *work_dir, int &topmemory, int mem_lmt,
                           int solution_id, int num_of_test, std::string &global_work_dir);
int compare_zoj(const char *file1, const char *file2, int DEBUG, int full_diff);
bool is_not_character(int c);
bool is_number(const std::string &s);
void move_to_next_nonspace_character(int &c, FILE *&f, int &ret);
void find_next_nonspace(int &c1, int &c2, FILE *&f1, FILE *&f2, int &ret, int DEBUG);
extern int choose;

extern "C" Language *createInstancebash();
extern "C" void destroyIntancebash(Language *language);
extern "C" Language *createInstancec11();
extern "C" void destroyInstancec11(Language *language);
extern "C" Language *createInstancec99();
extern "C" void destroyInstancec99(Language *language);
extern "C" Language *createInstanceclang();
extern "C" void destroyInstanceclang(Language *language);
extern "C" Language *createInstanceclang11();
extern "C" void destroyInstanceclang11(Language *language);
extern "C" Language *createInstanceclangpp();
extern "C" void destroyInstanceclangpp(Language *language);
extern "C" Language *createInstanceclangpp17();
extern "C" void destroyInstanceclangpp17(Language *language);
extern "C" Language *createInstancecpp11();
extern "C" void destroyInstancecpp11(Language *language);
extern "C" Language *createInstancecpp17();
extern "C" void destroyInstancecpp17(Language *language);
extern "C" Language *createInstancecpp20();
extern "C" void destroyInstancecpp20(Language *language);
extern "C" Language *createInstancecpp98();
extern "C" void destroyInstancecpp98(Language *language);
extern "C" Language *createInstancecsharp();
extern "C" void destroyInstancecsharp(Language *language);
extern "C" Language *createInstancefreebasic();
extern "C" void destroyInstancefreebasic(Language *language);
extern "C" Language *createInstancego();
extern "C" void destroyInstancego(Language *language);
extern "C" Language *createInstancejava();
extern "C" void destroyInstanceJava(Language *language);
extern "C" Language *createInstancejava6();
extern "C" void destroyInstancejava6(Language *language);
extern "C" Language *createInstancejava7();
extern "C" void destroyInstancejava7(Language *language);
extern "C" Language *createInstancejava8();
extern "C" void destroyInstancejava8(Language *language);
extern "C" Language *createInstancejavascript();
extern "C" void destroyInstancejavascript(Language *language);
extern "C" Language *createInstancekotlin();
extern "C" void destroyInstancekotlin(Language *language);
extern "C" Language *createInstancekotlinnative();
extern "C" void destroyInstancekotlinnative(Language *language);
extern "C" Language *createInstancelua();
extern "C" void destroyInstancelua(Language *language);
extern "C" Language *createInstanceobjectivec();
extern "C" void destroyInstanceobjectivec(Language *language);
extern "C" Language *createInstancepascal();
extern "C" void destroyInstancepascal(Language *language);
extern "C" Language *createInstanceperl();
extern "C" void destroyInstanceperl(Language *language);
extern "C" Language *createInstancephp();
extern "C" void destroyInstancephp(Language *language);
extern "C" Language *createInstancepypy();
extern "C" void destroyInstancepypy(Language *language);
extern "C" Language *createInstancepypy3();
extern "C" void destroyInstancepypy3(Language *language);
extern "C" Language *createInstancepython2();
extern "C" void destroyInstancepython2(Language *language);
extern "C" Language *createInstancepython3();
extern "C" void destroyInstancepython3(Language *language);
extern "C" Language *createInstanceruby();
extern "C" void destroyInstanceruby(Language *language);
extern "C" Language *createInstanceschema();
extern "C" void destroyInstanceschema(Language *language);

struct TestFailure {
    std::string msg;
};

using TestFunc = void (*)();
struct TestCase {
    const char *name;
    TestFunc func;
};

static std::vector<TestCase> &tests() {
    static std::vector<TestCase> t;
    return t;
}

static bool register_test(const char *name, TestFunc func) {
    tests().push_back({name, func});
    return true;
}

static void fail(const std::string &msg) {
    throw TestFailure{msg};
}

#define TEST(name) \
    void name();   \
    static bool name##_registered = register_test(#name, name); \
    void name()

#define EXPECT_TRUE(x) \
    do { \
        if (!(x)) { \
            fail(std::string("EXPECT_TRUE failed: ") + #x); \
        } \
    } while (0)

#define EXPECT_EQ(a, b) \
    do { \
        if (!((a) == (b))) { \
            std::ostringstream _oss; \
            _oss << "EXPECT_EQ failed: " << #a << " != " << #b; \
            fail(_oss.str()); \
        } \
    } while (0)

#define EXPECT_NE(a, b) \
    do { \
        if (!((a) != (b))) { \
            std::ostringstream _oss; \
            _oss << "EXPECT_NE failed: " << #a << " == " << #b; \
            fail(_oss.str()); \
        } \
    } while (0)

struct TempDir {
    std::filesystem::path path;
    TempDir() {
        auto base = std::filesystem::temp_directory_path();
        auto unique = std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count());
        path = base / ("cupjudge_test_" + unique);
        std::filesystem::create_directories(path);
    }
    ~TempDir() {
        std::error_code ec;
        std::filesystem::remove_all(path, ec);
    }
};

static void write_file(const std::filesystem::path &path, const std::string &data) {
    auto parent = path.parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent);
    }
    std::ofstream out(path, std::ios::binary);
    out << data;
}

static std::string read_file(const std::filesystem::path &path) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static void normalize_file_eol(const std::filesystem::path &path) {
    auto content = read_file(path);
    std::string out;
    out.reserve(content.size());
    for (size_t i = 0; i < content.size(); ++i) {
        char c = content[i];
        if (c == '\r') {
            if (i + 1 < content.size() && content[i + 1] == '\n') {
                ++i;
            }
            out.push_back('\n');
        } else {
            out.push_back(c);
        }
    }
    write_file(path, out);
}

static void copy_tree(const std::filesystem::path &src, const std::filesystem::path &dst) {
    std::filesystem::create_directories(dst);
    for (const auto &entry : std::filesystem::recursive_directory_iterator(src)) {
        auto rel = std::filesystem::relative(entry.path(), src);
        auto target = dst / rel;
        if (entry.is_directory()) {
            std::filesystem::create_directories(target);
        } else if (entry.is_regular_file()) {
            std::filesystem::create_directories(target.parent_path());
            std::filesystem::copy_file(entry.path(), target,
                                       std::filesystem::copy_options::overwrite_existing);
        }
    }
}

static void expect_exit(const std::function<void()> &fn, int code) {
    try {
        fn();
        fail("expected exit");
    } catch (const test_hooks::ExitException &ex) {
        EXPECT_EQ(ex.code, code);
    }
}

static void reset_globals_for_test() {
    DEBUG = 0;
    NO_RECORD = 0;
    MYSQL_MODE = true;
    READ_FROM_STDIN = false;
    javaTimeBonus = 5;
    java_memory_bonus = 512;
    std::memset(java_xms, 0, sizeof(java_xms));
    std::memset(java_xmx, 0, sizeof(java_xmx));
    sim_enable = 0;
    ALL_TEST_MODE = 1;
    full_diff = 0;
    use_max_time = 0;
    http_judge = 0;
    std::memset(http_baseurl, 0, sizeof(http_baseurl));
    std::memset(http_username, 0, sizeof(http_username));
    std::memset(http_password, 0, sizeof(http_password));
    SHARE_MEMORY_RUN = 0;
    record_call = 0;
    use_ptrace = 1;
    enable_parallel = 0;
    judger_number = 0;
    admin = false;
    no_sim = false;
    solution_id = 0;
    choose = 0;
}

static bool run_seccomp() {
    const char *val = std::getenv("RUN_SECCOMP");
    if (!val || !*val) {
        return false;
    }
    return std::strcmp(val, "0") != 0;
}

template <typename T>
static void exercise_seccomp_sandbox(T &lang) {
    lang.buildSeccompSandbox();
}

static void exercise_factory(Language *(*create_fn)(), void (*destroy_fn)(Language *)) {
    Language *lang = create_fn();
    EXPECT_TRUE(lang != nullptr);
    destroy_fn(lang);
}

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

TEST(PackBasics) {
    Pack p;
    EXPECT_EQ(p.getType(), UNDEFINED);
    Pack p2(3);
    EXPECT_EQ(p2.getInt(), 3);
    EXPECT_TRUE(p2.increment());
    EXPECT_EQ(p2.getInt(), 4);
    EXPECT_EQ(static_cast<int>(p2), 4);
    Pack p3(2.5);
    EXPECT_EQ(p3.getType(), DOUBLE);
    EXPECT_EQ(static_cast<double>(p3), 2.5);
    Pack p4(std::string("hi"));
    EXPECT_EQ(p4.getString(), "hi");
    EXPECT_EQ(static_cast<std::string>(p4), "hi");
}

TEST(EncodingHelpers) {
    EXPECT_EQ(from_hex('a'), 10);
    EXPECT_EQ(to_hex(15), 'f');
    char src[] = "a b+c";
    char *encoded = url_encode(src);
    EXPECT_EQ(std::string(encoded), "a+b%2bc");
    free(encoded);
    EXPECT_TRUE(utf8_check_is_valid("abc"));
    std::string invalid("\xff", 1);
    EXPECT_TRUE(!utf8_check_is_valid(invalid));
}

TEST(BundleBasics) {
    Bundle b;
    b.setSolutionID(1).setResult(ACCEPT).setFinished(FINISHED).setUsedTime(1.2);
    std::string json = b.toJSONString();
    EXPECT_TRUE(json.find("solution_id") != std::string::npos);
    b.clear();
    b.clear();
    std::string json2 = b.toJSONString();
    EXPECT_TRUE(json2.find("wid") != std::string::npos);
}

TEST(BundleStringFields) {
    Bundle b;
    std::string compile_info = "hello";
    b.setCompileInfo(compile_info);
    b.setResult(999);
    std::string long_str(9000, 'a');
    b.setTestRunResult(long_str);
    std::string json = b.toJSONString();
    EXPECT_TRUE(json.find("hello") != std::string::npos);
    EXPECT_TRUE(json.find("\"state\":16") != std::string::npos);
    EXPECT_TRUE(json.find("Omit") != std::string::npos);
}

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
    char infile[512], outfile[512], userfile[512];
    int pid = 1;
    prepare_files("1.in", 1, infile, pid,
                  work_dir.data(),
                  outfile, userfile, 2);
    EXPECT_TRUE(std::string(infile).find("/data/") != std::string::npos);
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
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(tmp.path);
    char code[CODESIZE] = {};
    getSolutionFromSubmissionInfo(info, code);
    EXPECT_TRUE(std::string(code).find("code") != std::string::npos);
    EXPECT_TRUE(std::filesystem::exists(tmp.path / "Main.txt"));
    std::filesystem::current_path(old_cwd);
}

TEST(WebSocketSenderBasics) {
    WebSocketSender sender;
    sender.connect("ws://127.0.0.1:1");
    EXPECT_TRUE(sender.isConnected());
    sender.emit("hello");
    WebSocketSender sender2("ws://127.0.0.1:1");
    EXPECT_TRUE(sender2.isConnected());
    sender2.send("ping");
}

TEST(WebSocketSenderAddressParsing) {
    WebSocketSender sender;
    sender.setAddressAndPort("127.0.0.1:1234");
    expect_exit([&sender] { sender.setAddressAndPort("127.0.0.1"); }, 1);
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
    char old_oj_home[BUFFER_SIZE] = {};
    std::strcpy(old_oj_home, oj_home);
    std::strcpy(oj_home, tmp.path.string().c_str());
    init_mysql_conf();
    EXPECT_EQ(std::string(host_name), "host");
    EXPECT_EQ(std::string(user_name), "user");
    EXPECT_EQ(std::string(password), "pass");
    EXPECT_EQ(std::string(db_name), "db");
    EXPECT_EQ(database_port, 3307);
    EXPECT_EQ(javaTimeBonus, 7);
    EXPECT_EQ(java_memory_bonus, 8);
    EXPECT_EQ(sim_enable, 1);
    EXPECT_EQ(full_diff, 1);
    EXPECT_EQ(SHARE_MEMORY_RUN, 1);
    EXPECT_EQ(use_max_time, 1);
    EXPECT_EQ(use_ptrace, 0);
    EXPECT_EQ(ALL_TEST_MODE, 1);
    EXPECT_EQ(enable_parallel, 1);
    std::strcpy(oj_home, old_oj_home);
}

TEST(BundleInvalidUtf8String) {
    Bundle b;
    std::string invalid("\xff", 1);
    b.setCompileInfo(invalid);
    std::string json = b.toJSONString();
    EXPECT_TRUE(json.find("检测到非法UTF-8输出") != std::string::npos);
}

TEST(BundleTrimLongString) {
    Bundle b;
    std::string long_str(9000, 'a');
    b.setRuntimeInfo(long_str);
    std::string json = b.toJSONString();
    EXPECT_TRUE(json.find("Omit") != std::string::npos);
}

TEST(ProcessAndLanguageHelpers) {
    test_hooks::reset();
    int rss = get_proc_status(getpid(), "VmRSS:");
    EXPECT_TRUE(rss >= 0);
    EXPECT_TRUE(isPython(PYTHON2));
    EXPECT_TRUE(!isPython(CPP11));
    setRunUser();
}

TEST(MySQLAutoPointerBasics) {
    test_hooks::reset();
    MySQLAutoPointer ptr;
    EXPECT_TRUE(ptr.setHostName("h"));
    EXPECT_TRUE(ptr.setUserName("u"));
    EXPECT_TRUE(ptr.setPassword("p"));
    EXPECT_TRUE(ptr.setDBName("d"));
    EXPECT_TRUE(ptr.setPort(3306));
    EXPECT_TRUE(ptr.setDebugMode(true));
    EXPECT_TRUE(ptr.start());
    EXPECT_TRUE(ptr.isConnected());
    test_hooks::state().mysql_query_results.push_back(1);
    test_hooks::state().mysql_error = "other error";
    EXPECT_EQ(ptr.query("select 1"), 1);
    test_hooks::state().mysql_query_results.push_back(0);
    EXPECT_EQ(ptr.query("select 1"), 0);
}

TEST(MySQLSubmissionInfoManagerBasics) {
    test_hooks::reset();
    test_hooks::push_mysql_result({{"1", "u", "0"}});
    test_hooks::push_mysql_result({{"1.5", "64", "0"}});
    test_hooks::push_mysql_result({{"code"}});
    test_hooks::push_mysql_result({{"input"}});
    MySQLSubmissionInfoManager mgr;
    mgr.setHostName("h").setUserName("u").setPassword("p").setDBName("d").setPort(1).start();
    EXPECT_TRUE(mgr.isConnected());
    int p_id = 0;
    int lang = 0;
    char user_id[64] = {};
    mgr.getSolutionInfo(1, p_id, user_id, lang);
    EXPECT_EQ(p_id, 1);
    double tl = 0;
    int ml = 0;
    int spj = 0;
    mgr.getProblemInfo(1, tl, ml, spj);
    EXPECT_EQ(ml, 64);
    char code[128] = {};
    TempDir tmp;
    std::string work_dir = tmp.path.string();
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(tmp.path);
    mgr.getSolution(1, work_dir.data(), 0, code, "txt", 0);
    EXPECT_TRUE(std::string(code).find("code") != std::string::npos);
    mgr.getCustomInput(1, work_dir.data());
    EXPECT_EQ(read_file(tmp.path / "data.in"), "input");
    std::filesystem::current_path(old_cwd);
}

TEST(CompareImplBasics) {
    TempDir tmp;
    auto a = tmp.path / "a.out";
    auto b = tmp.path / "b.out";
    write_file(a, "1\n");
    write_file(b, "2\n");
    CompareImpl cmp;
    cmp.setDebug(0);
    cmp.setFullDiff(0);
    int res = cmp.compare(a.string().c_str(), b.string().c_str());
    EXPECT_EQ(res, WRONG_ANSWER);
}

TEST(LanguageClassesBasics) {
    C11 c11;
    C99 c99;
    Clang clang;
    Clang11 clang11;
    Clangpp clangpp;
    Clangpp17 clangpp17;
    Cpp98 cpp98;
    Cpp11 cpp11;
    Cpp17 cpp17;
    Cpp20 cpp20;
    Bash bash;
    Go go;
    Java java;
    Java6 java6;
    Java7 java7;
    Java8 java8;
    Kotlin kotlin;
    KotlinNative kotlin_native;
    Python2 py2;
    Python3 py3;
    PyPy pypy;
    PyPy3 pypy3;
    Php php;
    Perl perl;
    Ruby ruby;
    Lua lua;
    Schema schema;
    JavaScript js;
    Csharp cs;
    Objc objc;
    FreeBasic fb;
    Pascal pas;
    int counter[512] = {};
    c11.initCallCounter(counter);
    EXPECT_TRUE(counter[0] == HOJ_MAX_LIMIT);
    C11 &cpp98_as_c11 = cpp98;
    EXPECT_EQ(cpp98_as_c11.getFileSuffix(), "cc");
    EXPECT_EQ(py2.getFileSuffix(), "py");
    EXPECT_EQ(java.getFileSuffix(), "java");
    EXPECT_EQ(js.getFileSuffix(), "js");
    if (run_seccomp()) {
        bash.buildSeccompSandbox();
        java.buildSeccompSandbox();
        py3.buildSeccompSandbox();
        kotlin_native.buildSeccompSandbox();
        php.buildSeccompSandbox();
        schema.buildSeccompSandbox();
        cs.buildSeccompSandbox();
        objc.buildSeccompSandbox();
        fb.buildSeccompSandbox();
        pas.buildSeccompSandbox();
        ruby.buildSeccompSandbox();
        perl.buildSeccompSandbox();
        lua.buildSeccompSandbox();
        go.buildSeccompSandbox();
        clang.buildSeccompSandbox();
        clang11.buildSeccompSandbox();
        clangpp.buildSeccompSandbox();
        clangpp17.buildSeccompSandbox();
    }
}

TEST(SpecialJudgeBasics) {
    test_hooks::reset();
    test_hooks::state().pipe_seed = "0";
    TempDir tmp;
    auto data_dir = tmp.path / "data" / "1";
    std::filesystem::create_directories(data_dir);
    char oj_home_path[1024] = {};
    std::strcpy(oj_home_path, tmp.path.string().c_str());
    char infile[] = "in";
    char outfile[] = "out";
    char userfile[] = "user";
    char code[] = "code";
    std::string work = tmp.path.string() + "/";
    int stdout_fd = dup(fileno(stdout));
    int res = SpecialJudge::newInstance().setDebug(false).run(
        oj_home_path, 1, infile, outfile, userfile, code, work);
    if (stdout_fd >= 0) {
        fflush(stdout);
        dup2(stdout_fd, fileno(stdout));
        close(stdout_fd);
    }
    EXPECT_EQ(res, ACCEPT);
}

TEST(SpecialJudgePipeSeedBranches) {
    test_hooks::reset();
    TempDir tmp;
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(tmp.path);
    auto userfile = tmp.path / "user";
    write_file(userfile, "1\n");
    char oj_home_path[1024] = {};
    std::strcpy(oj_home_path, tmp.path.string().c_str());
    char infile[] = "in";
    char outfile[] = "out";
    char userfile_buf[64] = {};
    std::strcpy(userfile_buf, userfile.string().c_str());
    char code[] = "code";
    std::string work = tmp.path.string() + "/";

    int stdout_fd = dup(fileno(stdout));
    test_hooks::state().fork_results.push_back(1);
    test_hooks::state().pipe_seed = "1";
    int res = SpecialJudge::newInstance().setDebug(true).run(
        oj_home_path, 1, infile, outfile, userfile_buf, code, work);
    EXPECT_EQ(res, WRONG_ANSWER);
    if (stdout_fd >= 0) {
        fflush(stdout);
        dup2(stdout_fd, fileno(stdout));
        close(stdout_fd);
    }

    stdout_fd = dup(fileno(stdout));
    test_hooks::reset();
    test_hooks::state().fork_results.push_back(1);
    test_hooks::state().pipe_seed = "0";
    res = SpecialJudge::newInstance().setDebug(false).run(
        oj_home_path, 1, infile, outfile, userfile_buf, code, work);
    EXPECT_EQ(res, ACCEPT);
    if (stdout_fd >= 0) {
        fflush(stdout);
        dup2(stdout_fd, fileno(stdout));
        close(stdout_fd);
    }

    stdout_fd = dup(fileno(stdout));
    test_hooks::reset();
    test_hooks::state().fork_results.push_back(1);
    test_hooks::state().pipe_seed = "6";
    res = SpecialJudge::newInstance().setDebug(false).run(
        oj_home_path, 1, infile, outfile, userfile_buf, code, work);
    EXPECT_EQ(res, 6);
    if (stdout_fd >= 0) {
        fflush(stdout);
        dup2(stdout_fd, fileno(stdout));
        close(stdout_fd);
    }

    std::filesystem::current_path(old_cwd);
}

TEST(SpecialJudgePipeFailure) {
    test_hooks::reset();
    TempDir tmp;
    char oj_home_path[1024] = {};
    std::strcpy(oj_home_path, tmp.path.string().c_str());
    char infile[] = "in";
    char outfile[] = "out";
    char userfile[] = "user";
    char code[] = "code";
    std::string work = tmp.path.string() + "/";
    test_hooks::state().pipe_fail = true;
    int res = SpecialJudge::newInstance().setDebug(false).run(
        oj_home_path, 1, infile, outfile, userfile, code, work);
    EXPECT_EQ(res, 16);
}

TEST(SpecialJudgeChildSpj) {
    test_hooks::reset();
    TempDir tmp;
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(tmp.path);
    auto data_dir = tmp.path / "data" / "1";
    write_file(data_dir / "spj", "");
    write_file(tmp.path / "user.out", "u\n");
    write_file(tmp.path / "diff.out", "diff\n");
    char oj_home_path[1024] = {};
    std::strcpy(oj_home_path, tmp.path.string().c_str());
    char infile[] = "in";
    char outfile[] = "out";
    char userfile[512] = {};
    std::snprintf(userfile, sizeof(userfile), "%s", (tmp.path / "user.out").string().c_str());
    char code[] = "code";
    std::string work = tmp.path.string() + "/";
    test_hooks::state().fork_results.push_back(0);
    test_hooks::state().pipe_keep_open = true;
    test_hooks::state().system_result = 1 << 8;
    test_hooks::state().exit_throws = false;
    int stdout_fd = dup(fileno(stdout));
    int res = SpecialJudge::newInstance().setDebug(true).run(
        oj_home_path, 1, infile, outfile, userfile, code, work);
    EXPECT_EQ(test_hooks::state().last_exit_code, 0);
    EXPECT_EQ(res, WRONG_ANSWER);
    test_hooks::state().exit_throws = true;
    if (stdout_fd >= 0) {
        fflush(stdout);
        dup2(stdout_fd, fileno(stdout));
        close(stdout_fd);
    }
    if (test_hooks::state().pipe_keep_fd >= 0) {
        close(test_hooks::state().pipe_keep_fd);
        test_hooks::state().pipe_keep_fd = -1;
    }
    test_hooks::state().pipe_keep_open = false;
    std::filesystem::current_path(old_cwd);
}

TEST(SpecialJudgeChildSpjScripts) {
    test_hooks::reset();
    TempDir tmp_js;
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(tmp_js.path);
    auto data_dir_js = tmp_js.path / "data" / "1";
    write_file(data_dir_js / "spj.js", "");
    write_file(tmp_js.path / "user.out", "u\n");
    char oj_home_path_js[1024] = {};
    std::strcpy(oj_home_path_js, tmp_js.path.string().c_str());
    char infile[] = "in";
    char outfile[] = "out";
    char userfile_js[512] = {};
    std::snprintf(userfile_js, sizeof(userfile_js), "%s",
                  (tmp_js.path / "user.out").string().c_str());
    char code[] = "code";
    std::string work_js = tmp_js.path.string() + "/";
    test_hooks::state().fork_results.push_back(0);
    test_hooks::state().pipe_keep_open = true;
    test_hooks::state().system_result = 1;
    test_hooks::state().exit_throws = false;
    int stdout_fd = dup(fileno(stdout));
    int res_js = SpecialJudge::newInstance().setDebug(false).run(
        oj_home_path_js, 1, infile, outfile, userfile_js, code, work_js);
    EXPECT_EQ(test_hooks::state().last_exit_code, 0);
    EXPECT_EQ(res_js, ACCEPT);
    test_hooks::state().exit_throws = true;
    if (stdout_fd >= 0) {
        fflush(stdout);
        dup2(stdout_fd, fileno(stdout));
        close(stdout_fd);
    }
    if (test_hooks::state().pipe_keep_fd >= 0) {
        close(test_hooks::state().pipe_keep_fd);
        test_hooks::state().pipe_keep_fd = -1;
    }
    test_hooks::state().pipe_keep_open = false;
    std::filesystem::current_path(old_cwd);

    test_hooks::reset();
    TempDir tmp_py;
    std::filesystem::current_path(tmp_py.path);
    auto data_dir_py = tmp_py.path / "data" / "1";
    write_file(data_dir_py / "spj.py", "");
    write_file(tmp_py.path / "user.out", "u\n");
    char oj_home_path_py[1024] = {};
    std::strcpy(oj_home_path_py, tmp_py.path.string().c_str());
    char userfile_py[512] = {};
    std::snprintf(userfile_py, sizeof(userfile_py), "%s",
                  (tmp_py.path / "user.out").string().c_str());
    std::string work_py = tmp_py.path.string() + "/";
    test_hooks::state().fork_results.push_back(0);
    test_hooks::state().pipe_keep_open = true;
    test_hooks::state().system_result = 0;
    test_hooks::state().exit_throws = false;
    stdout_fd = dup(fileno(stdout));
    int res_py = SpecialJudge::newInstance().setDebug(false).run(
        oj_home_path_py, 1, infile, outfile, userfile_py, code, work_py);
    EXPECT_EQ(test_hooks::state().last_exit_code, 0);
    EXPECT_EQ(res_py, ACCEPT);
    test_hooks::state().exit_throws = true;
    if (stdout_fd >= 0) {
        fflush(stdout);
        dup2(stdout_fd, fileno(stdout));
        close(stdout_fd);
    }
    if (test_hooks::state().pipe_keep_fd >= 0) {
        close(test_hooks::state().pipe_keep_fd);
        test_hooks::state().pipe_keep_fd = -1;
    }
    test_hooks::state().pipe_keep_open = false;
    std::filesystem::current_path(old_cwd);
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

TEST(BundleAdditionalFields) {
    Bundle b;
    char judger[] = "judge";
    b.setJudger(judger);
    std::string jid = "jid";
    b.setJudgerId(jid);
    std::string runtime("\xff", 1);
    b.setRuntimeInfo(runtime);
    b.setResult(999);
    b.setSim(1).setSimSource(2).setTotalPoint(3);
    std::string json = b.toJSONString();
    EXPECT_TRUE(json.find("\"state\":16") != std::string::npos);
    EXPECT_TRUE(json.find("runtime_info") != std::string::npos);
}

TEST(BundleJudgerStringOverload) {
    Bundle b;
    std::string judger = "judge";
    b.setJudger(judger);
    std::string out = static_cast<std::string>(b);
    EXPECT_TRUE(out.find("judge") != std::string::npos);
}

TEST(PackNonIntIncrement) {
    Pack p(std::string("x"));
    EXPECT_TRUE(!p.increment());
    EXPECT_TRUE(p.isString());
    Pack p2(1.5);
    EXPECT_TRUE(p2.isFloat());
    EXPECT_EQ(static_cast<int>(p2), 0);
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

TEST(WebSocketSenderParsing) {
    WebSocketSender sender;
    sender.setAddressAndPort("127.0.0.1:8080");
    sender.setPort(1234);
    sender.setAddress("ws://example");
    expect_exit([&]() { sender.setAddressAndPort("bad"); }, 1);
}

#ifdef UNIT_TEST
#undef seccomp_load
#endif

TEST(SeccompStubCoverage) {
    scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_ALLOW);
    EXPECT_TRUE(ctx != nullptr);
    EXPECT_EQ(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, 0, 0), 0);
    EXPECT_EQ(seccomp_load(ctx), 0);
}

#ifdef UNIT_TEST
#define seccomp_load test_seccomp_load
#endif

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

TEST(EasyWsClientStub) {
    auto *ws = easywsclient::WebSocket::from_url("ws://example", "");
    EXPECT_TRUE(ws != nullptr);
    ws->poll(1);
    ws->send("hi");
    ws->sendBinary(std::string("bin"));
    std::vector<uint8_t> bytes = {1, 2, 3};
    ws->sendBinary(bytes);
    ws->sendPing();
    EXPECT_EQ(ws->getReadyState(), easywsclient::WebSocket::OPEN);
    ws->close();
    delete ws;

    auto *ws2 = easywsclient::WebSocket::from_url_no_mask("ws://example", "");
    EXPECT_TRUE(ws2 != nullptr);
    ws2->close();
    delete ws2;

    auto *dummy = easywsclient::WebSocket::create_dummy();
    dummy->poll(0);
    dummy->close();
}

struct BonusLimitTester : BonusLimit {
    double time(double timeLimit, double bonus) {
        return buildBonusTimeLimit(timeLimit, bonus);
    }
    int memory(int memoryLimit, int bonus) {
        return buildBonusMemoryLimit(memoryLimit, bonus);
    }
};

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

struct DummyLanguage : Language {
    void run(int memory) override {
        (void)memory;
    }
    void initCallCounter(int *call_counter) override {
        if (call_counter) {
            call_counter[0] = HOJ_MAX_LIMIT;
        }
    }
    std::string getFileSuffix() override {
        return "txt";
    }
};

struct C11Expose : C11 {
    using C11::getArgs;
};

struct JavaExpose : Java {
    using Java::getArgs;
};

struct KotlinNativeExpose : KotlinNative {
    using KotlinNative::getArgs;
};

struct CsharpExpose : Csharp {
    using Csharp::getArgs;
};

template <typename T>
static void exercise_python_like(T &lang, const std::filesystem::path &root,
                                 int expected_compile_result) {
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(root);
    write_file(root / "error.out", "SyntaxError");
    write_file(root / "ce.txt", "err");
    int calls[call_array_size] = {};
    lang.initCallCounter(calls);
    EXPECT_EQ(lang.buildTimeLimit(1.0, 2.0), 4.0);
    EXPECT_EQ(lang.buildMemoryLimit(64, 10), 74);
    EXPECT_EQ(lang.getCompileResult(0), expected_compile_result);
    EXPECT_EQ(lang.fixACStatus(ACCEPT), RUNTIME_ERROR);
    int ac = ACCEPT;
    lang.fixACFlag(ac);
    EXPECT_EQ(ac, RUNTIME_ERROR);
    int top = 0;
    test_hooks::state().system_result = 0;
    std::string work = root.string();
    lang.fixFlagWithVMIssue(work.data(), ac, top, 64);
    EXPECT_EQ(ac, MEMORY_LIMIT_EXCEEDED);
    EXPECT_EQ(top, 64 * STD_MB);
    rusage ruse{};
    rlimit lim{};
    EXPECT_TRUE(lang.getMemory(ruse, getpid()) >= 0);
    lang.runMemoryLimit(lim);
    lang.buildRuntime(work.c_str());
    if (run_seccomp()) {
        lang.buildSeccompSandbox();
    }
    lang.run(64);
    EXPECT_TRUE(!test_hooks::state().last_exec_path.empty());
    std::filesystem::current_path(old_cwd);
}

TEST(LanguageBaseBasics) {
    test_hooks::reset();
    DummyLanguage lang;
    TempDir tmp;
    int calls[call_array_size] = {};
    lang.initCallCounter(calls);
    EXPECT_EQ(calls[0], HOJ_MAX_LIMIT);
    lang.setProcessLimit();
    lang.setCompileProcessLimit();
    lang.setCompileMount(tmp.path.string().c_str());
    lang.setCompileExtraConfig();
    lang.buildRuntime(tmp.path.string().c_str());
    EXPECT_EQ(lang.buildTimeLimit(1.5, 2.0), 1.5);
    EXPECT_EQ(lang.buildMemoryLimit(64, 10), 64);
    lang.buildChrootSandbox(tmp.path.string().c_str());
    rlimit lim{};
    lang.runMemoryLimit(lim);
    int ac = ACCEPT;
    lang.fixACFlag(ac);
    int top = 0;
    lang.fixFlagWithVMIssue(const_cast<char *>(tmp.path.string().c_str()), ac, top, 64);
    EXPECT_TRUE(lang.supportParallel());
    EXPECT_TRUE(lang.isValidExitCode(0));
    EXPECT_TRUE(!lang.isValidExitCode(1));
    lang.setDebug(1);
}

TEST(LanguagePythonFamily) {
    test_hooks::reset();
    TempDir tmp;
    Python2 py2;
    Python3 py3;
    PyPy pypy;
    PyPy3 pypy3;
    exercise_python_like(py2, tmp.path, 3);
    exercise_python_like(py3, tmp.path, 3);
    exercise_python_like(pypy, tmp.path, 0);
    exercise_python_like(pypy3, tmp.path, 0);
}

TEST(LanguagePythonNoErrorBranches) {
    test_hooks::reset();
    TempDir tmp;
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(tmp.path);
    write_file(tmp.path / "error.out", "");
    Python2 py2;
    Python3 py3;
    PyPy pypy;
    PyPy3 pypy3;
    EXPECT_EQ(py2.fixACStatus(ACCEPT), ACCEPT);
    EXPECT_EQ(py3.fixACStatus(ACCEPT), ACCEPT);
    EXPECT_EQ(pypy.fixACStatus(ACCEPT), ACCEPT);
    EXPECT_EQ(pypy3.fixACStatus(ACCEPT), ACCEPT);
    int ac = ACCEPT;
    py2.fixACFlag(ac);
    EXPECT_EQ(ac, ACCEPT);
    py3.fixACFlag(ac);
    EXPECT_EQ(ac, ACCEPT);
    pypy.fixACFlag(ac);
    EXPECT_EQ(ac, ACCEPT);
    pypy3.fixACFlag(ac);
    EXPECT_EQ(ac, ACCEPT);
    write_file(tmp.path / "error.out", "OtherError");
    EXPECT_EQ(py2.fixACStatus(ACCEPT), ACCEPT);
    EXPECT_EQ(py3.fixACStatus(ACCEPT), ACCEPT);
    EXPECT_EQ(pypy.fixACStatus(ACCEPT), ACCEPT);
    EXPECT_EQ(pypy3.fixACStatus(ACCEPT), ACCEPT);
    ac = ACCEPT;
    py2.fixACFlag(ac);
    EXPECT_EQ(ac, ACCEPT);
    py3.fixACFlag(ac);
    EXPECT_EQ(ac, ACCEPT);
    pypy.fixACFlag(ac);
    EXPECT_EQ(ac, ACCEPT);
    pypy3.fixACFlag(ac);
    EXPECT_EQ(ac, ACCEPT);
    std::filesystem::current_path(old_cwd);
}

TEST(LanguageC11Extra) {
    test_hooks::reset();
    C11Expose c11;
    rlimit lim{};
    c11.runMemoryLimit(lim);
    c11.setCompileMount("work");
    EXPECT_TRUE(c11.enableSim());
    EXPECT_TRUE(c11.gotErrorWhileRunning(true));
    if (run_seccomp()) {
        c11.buildSeccompSandbox();
    }
    c11.run(64);
    EXPECT_EQ(test_hooks::state().last_exec_path, "./Main");
    EXPECT_TRUE(c11.getArgs() != nullptr);
}

TEST(LanguageJavaFamily) {
    test_hooks::reset();
    TempDir tmp;
    std::string work = tmp.path.string();
    std::strcpy(oj_home, work.c_str());
    write_file(tmp.path / "etc" / "java0.policy", "policy");
    JavaExpose java;
    std::vector<std::string> args = {"javac", "Main.java"};
    java.compile(args, "-Xms32m", "-Xmx256m");
    EXPECT_EQ(test_hooks::state().last_exec_path, "javac");
    java.run(64);
    EXPECT_EQ(test_hooks::state().last_exec_path, "/usr/bin/java");
    java.setProcessLimit();
    java.setCompileProcessLimit();
    java.buildRuntime(work.c_str());
    EXPECT_EQ(java.buildTimeLimit(1.0, 2.0), 4.0);
    EXPECT_EQ(java.buildMemoryLimit(64, 10), 74);
    java.setCompileMount(work.c_str());
    java.setExtraPolicy(oj_home, work.c_str());
    int calls[call_array_size] = {};
    java.initCallCounter(calls);
    EXPECT_EQ(java.getFileSuffix(), "java");
    EXPECT_TRUE(java.getArgs() != nullptr);
    EXPECT_TRUE(java.enableSim());
    int ac = ACCEPT;
    int top = 0;
    test_hooks::state().system_result = 0;
    java.fixFlagWithVMIssue(work.data(), ac, top, 64);
    EXPECT_TRUE(java.isValidExitCode(17));
    EXPECT_TRUE(java.gotErrorWhileRunning(true));
    if (run_seccomp()) {
        java.buildSeccompSandbox();
    }
    java.buildChrootSandbox(work.c_str());
    Java6 java6;
    java6.run(32);
    EXPECT_EQ(test_hooks::state().last_exec_path, "/usr/bin/java-6");
    Java7 java7;
    java7.run(32);
    EXPECT_EQ(test_hooks::state().last_exec_path, "/usr/bin/java-7");
    Java8 java8;
    java8.run(32);
    EXPECT_EQ(test_hooks::state().last_exec_path, "/usr/bin/java-8");
}

TEST(LanguageFileSuffixes) {
    std::unique_ptr<Language> clangpp(createInstanceclangpp());
    std::unique_ptr<Language> clangpp17(createInstanceclangpp17());
    std::unique_ptr<Language> cpp17(createInstancecpp17());
    std::unique_ptr<Language> cpp20(createInstancecpp20());
    EXPECT_EQ(clangpp->getFileSuffix(), "cc");
    EXPECT_EQ(clangpp17->getFileSuffix(), "cc");
    EXPECT_EQ(cpp17->getFileSuffix(), "cc");
    EXPECT_EQ(cpp20->getFileSuffix(), "cc");
}

TEST(LanguageKotlinFamily) {
    test_hooks::reset();
    Kotlin kotlin;
    std::vector<std::string> args = {"kotlinc", "Main.kt"};
    kotlin.compile(args, "-Xms32m", "-Xmx256m");
    EXPECT_EQ(test_hooks::state().last_exec_path, "kotlinc");
    std::vector<std::string> empty_args;
    expect_exit([&]() { kotlin.compile(empty_args, "", ""); }, 0);
    kotlin.run(64);
    EXPECT_EQ(test_hooks::state().last_exec_path, "/usr/bin/java");
    EXPECT_EQ(kotlin.getFileSuffix(), "kt");
    EXPECT_TRUE(!kotlin.supportParallel());
    KotlinNativeExpose native;
    int calls[call_array_size] = {};
    native.initCallCounter(calls);
    EXPECT_EQ(native.getFileSuffix(), "kt");
    native.setProcessLimit();
    native.setCompileProcessLimit();
    native.setCompileMount("work");
    native.buildRuntime("work");
    EXPECT_EQ(native.buildTimeLimit(1.0, 2.0), 4.0);
    EXPECT_EQ(native.buildMemoryLimit(64, 10), 74);
    EXPECT_TRUE(native.getArgs() != nullptr);
    native.run(64);
    if (run_seccomp()) {
        native.buildSeccompSandbox();
    }
    EXPECT_TRUE(native.gotErrorWhileRunning(true));
}

TEST(LanguageOtherRuntimes) {
    test_hooks::reset();
    TempDir tmp;
    int stdout_fd = dup(fileno(stdout));
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(tmp.path);
    write_file(tmp.path / "ce.txt", "err");
    std::string work = tmp.path.string();
    Bash bash;
    int calls[call_array_size] = {};
    bash.initCallCounter(calls);
    EXPECT_EQ(bash.buildTimeLimit(1.0, 2.0), 4.0);
    EXPECT_EQ(bash.buildMemoryLimit(64, 10), 74);
    EXPECT_EQ(bash.getCompileResult(0), 3);
    bash.setProcessLimit();
    bash.buildRuntime(work.c_str());
    if (run_seccomp()) {
        bash.buildSeccompSandbox();
    }
    rusage ruse{};
    rlimit lim{};
    EXPECT_TRUE(bash.getMemory(ruse, getpid()) >= 0);
    EXPECT_TRUE(bash.gotErrorWhileRunning(true));
    bash.run(64);
    EXPECT_EQ(bash.getFileSuffix(), "sh");

    Go go;
    go.initCallCounter(calls);
    go.setProcessLimit();
    go.setCompileProcessLimit();
    EXPECT_EQ(go.buildTimeLimit(1.0, 2.0), 4.0);
    EXPECT_EQ(go.buildMemoryLimit(64, 10), 74);
    go.runMemoryLimit(lim);
    EXPECT_TRUE(!go.gotErrorWhileRunning(false));
    EXPECT_TRUE(!go.enableSim());
    EXPECT_EQ(go.getFileSuffix(), "go");
    if (run_seccomp()) {
        go.buildSeccompSandbox();
    }

    Lua lua;
    lua.initCallCounter(calls);
    lua.buildRuntime(work.c_str());
    EXPECT_EQ(lua.buildTimeLimit(1.0, 2.0), 4.0);
    EXPECT_EQ(lua.buildMemoryLimit(64, 10), 74);
    EXPECT_TRUE(lua.getMemory(ruse, getpid()) >= 0);
    if (run_seccomp()) {
        lua.buildSeccompSandbox();
    }
    lua.run(64);
    EXPECT_EQ(lua.getFileSuffix(), "lua");

    JavaScript js;
    js.initCallCounter(calls);
    js.setProcessLimit();
    js.buildRuntime(work.c_str());
    EXPECT_EQ(js.buildTimeLimit(1.0, 2.0), 4.0);
    EXPECT_EQ(js.buildMemoryLimit(64, 10), 74);
    if (run_seccomp()) {
        js.buildSeccompSandbox();
    }
    js.run(64);
    EXPECT_EQ(js.getFileSuffix(), "js");

    CsharpExpose cs;
    cs.initCallCounter(calls);
    cs.buildRuntime(work.c_str());
    cs.setProcessLimit();
    EXPECT_EQ(cs.buildTimeLimit(1.0, 2.0), 4.0);
    EXPECT_EQ(cs.buildMemoryLimit(64, 10), 74);
    EXPECT_TRUE(cs.getArgs() != nullptr);
    if (run_seccomp()) {
        cs.buildSeccompSandbox();
    }
    cs.run(64);
    EXPECT_TRUE(cs.gotErrorWhileRunning(true));
    EXPECT_EQ(cs.getFileSuffix(), "cs");

    Php php;
    php.initCallCounter(calls);
    php.buildRuntime(work.c_str());
    EXPECT_EQ(php.buildTimeLimit(1.0, 2.0), 4.0);
    EXPECT_EQ(php.buildMemoryLimit(64, 10), 74);
    if (run_seccomp()) {
        php.buildSeccompSandbox();
    }
    php.run(64);
    EXPECT_EQ(php.getFileSuffix(), "php");

    Perl perl;
    perl.initCallCounter(calls);
    perl.buildRuntime(work.c_str());
    EXPECT_EQ(perl.buildTimeLimit(1.0, 2.0), 4.0);
    EXPECT_EQ(perl.buildMemoryLimit(64, 10), 74);
    EXPECT_TRUE(perl.getMemory(ruse, getpid()) >= 0);
    if (run_seccomp()) {
        perl.buildSeccompSandbox();
    }
    perl.run(64);
    EXPECT_EQ(perl.getFileSuffix(), "pl");

    Ruby ruby;
    ruby.initCallCounter(calls);
    ruby.setProcessLimit();
    ruby.buildRuntime(work.c_str());
    EXPECT_EQ(ruby.buildTimeLimit(1.0, 2.0), 4.0);
    EXPECT_EQ(ruby.buildMemoryLimit(64, 10), 74);
    EXPECT_EQ(ruby.getCompileResult(0), 3);
    EXPECT_TRUE(ruby.getMemory(ruse, getpid()) >= 0);
    if (run_seccomp()) {
        ruby.buildSeccompSandbox();
    }
    ruby.run(64);
    EXPECT_EQ(ruby.getFileSuffix(), "rb");

    Schema schema;
    schema.initCallCounter(calls);
    schema.setProcessLimit();
    schema.buildRuntime(work.c_str());
    EXPECT_EQ(schema.buildTimeLimit(1.0, 2.0), 4.0);
    EXPECT_EQ(schema.buildMemoryLimit(64, 10), 74);
    EXPECT_TRUE(schema.getMemory(ruse, getpid()) >= 0);
    if (run_seccomp()) {
        schema.buildSeccompSandbox();
    }
    schema.run(64);
    EXPECT_EQ(schema.getFileSuffix(), "scm");

    Objc objc;
    objc.initCallCounter(calls);
    objc.buildRuntime(work.c_str());
    if (run_seccomp()) {
        objc.buildSeccompSandbox();
    }
    EXPECT_TRUE(objc.getMemory(ruse, getpid()) >= 0);
    objc.run(64);
    EXPECT_EQ(objc.getFileSuffix(), "m");

    FreeBasic fb;
    fb.initCallCounter(calls);
    fb.buildRuntime(work.c_str());
    fb.setCompileMount(work.c_str());
    EXPECT_EQ(fb.buildTimeLimit(1.0, 2.0), 4.0);
    EXPECT_EQ(fb.buildMemoryLimit(64, 10), 74);
    fb.setCompileExtraConfig();
    freopen("/dev/stdout", "w", stdout);
    fb.runMemoryLimit(lim);
    EXPECT_TRUE(fb.getMemory(ruse, getpid()) >= 0);
    EXPECT_TRUE(!fb.gotErrorWhileRunning(false));
    EXPECT_TRUE(!fb.enableSim());
    if (run_seccomp()) {
        fb.buildSeccompSandbox();
    }
    fb.run(64);
    EXPECT_EQ(fb.getFileSuffix(), "bas");

    Pascal pas;
    pas.initCallCounter(calls);
    pas.buildRuntime(work.c_str());
    pas.setCompileExtraConfig();
    freopen("/dev/stdout", "w", stdout);
    EXPECT_TRUE(pas.getMemory(ruse, getpid()) >= 0);
    if (run_seccomp()) {
        pas.buildSeccompSandbox();
    }
    pas.run(64);
    EXPECT_EQ(pas.getFileSuffix(), "pas");
    std::filesystem::current_path(old_cwd);
    if (stdout_fd >= 0) {
        fflush(stdout);
        dup2(stdout_fd, fileno(stdout));
        close(stdout_fd);
    }
}

TEST(LanguageSeccompSandboxes) {
    test_hooks::reset();
    Bash bash;
    C11 c11;
    Csharp cs;
    FreeBasic fb;
    Go go;
    Java java;
    JavaScript js;
    KotlinNative kn;
    Lua lua;
    Objc objc;
    Pascal pas;
    Perl perl;
    Php php;
    PyPy pypy;
    PyPy3 pypy3;
    Python2 py2;
    Python3 py3;
    Ruby ruby;
    Schema schema;
    Clang clang;
    Clang11 clang11;
    Clangpp clangpp;
    Clangpp17 clangpp17;
    Cpp11 cpp11;
    Cpp17 cpp17;
    Cpp20 cpp20;
    Cpp98 cpp98;
    C99 c99;
    exercise_seccomp_sandbox(bash);
    exercise_seccomp_sandbox(c11);
    exercise_seccomp_sandbox(cs);
    exercise_seccomp_sandbox(fb);
    exercise_seccomp_sandbox(go);
    exercise_seccomp_sandbox(java);
    exercise_seccomp_sandbox(js);
    exercise_seccomp_sandbox(kn);
    exercise_seccomp_sandbox(lua);
    exercise_seccomp_sandbox(objc);
    exercise_seccomp_sandbox(pas);
    exercise_seccomp_sandbox(perl);
    exercise_seccomp_sandbox(php);
    exercise_seccomp_sandbox(pypy);
    exercise_seccomp_sandbox(pypy3);
    exercise_seccomp_sandbox(py2);
    exercise_seccomp_sandbox(py3);
    exercise_seccomp_sandbox(ruby);
    exercise_seccomp_sandbox(schema);
    exercise_seccomp_sandbox(clang);
    exercise_seccomp_sandbox(clang11);
    exercise_seccomp_sandbox(clangpp);
    exercise_seccomp_sandbox(clangpp17);
    exercise_seccomp_sandbox(cpp11);
    exercise_seccomp_sandbox(cpp17);
    exercise_seccomp_sandbox(cpp20);
    exercise_seccomp_sandbox(cpp98);
    exercise_seccomp_sandbox(c99);
}

TEST(LanguageFactoryFunctions) {
    exercise_factory(createInstancebash, destroyIntancebash);
    exercise_factory(createInstancec11, destroyInstancec11);
    exercise_factory(createInstancec99, destroyInstancec99);
    exercise_factory(createInstanceclang, destroyInstanceclang);
    exercise_factory(createInstanceclang11, destroyInstanceclang11);
    exercise_factory(createInstanceclangpp, destroyInstanceclangpp);
    exercise_factory(createInstanceclangpp17, destroyInstanceclangpp17);
    exercise_factory(createInstancecpp11, destroyInstancecpp11);
    exercise_factory(createInstancecpp17, destroyInstancecpp17);
    exercise_factory(createInstancecpp20, destroyInstancecpp20);
    exercise_factory(createInstancecpp98, destroyInstancecpp98);
    exercise_factory(createInstancecsharp, destroyInstancecsharp);
    exercise_factory(createInstancefreebasic, destroyInstancefreebasic);
    exercise_factory(createInstancego, destroyInstancego);
    exercise_factory(createInstancejava, destroyInstanceJava);
    exercise_factory(createInstancejava6, destroyInstancejava6);
    exercise_factory(createInstancejava7, destroyInstancejava7);
    exercise_factory(createInstancejava8, destroyInstancejava8);
    exercise_factory(createInstancejavascript, destroyInstancejavascript);
    exercise_factory(createInstancekotlin, destroyInstancekotlin);
    exercise_factory(createInstancekotlinnative, destroyInstancekotlinnative);
    exercise_factory(createInstancelua, destroyInstancelua);
    exercise_factory(createInstanceobjectivec, destroyInstanceobjectivec);
    exercise_factory(createInstancepascal, destroyInstancepascal);
    exercise_factory(createInstanceperl, destroyInstanceperl);
    exercise_factory(createInstancephp, destroyInstancephp);
    exercise_factory(createInstancepypy, destroyInstancepypy);
    exercise_factory(createInstancepypy3, destroyInstancepypy3);
    exercise_factory(createInstancepython2, destroyInstancepython2);
    exercise_factory(createInstancepython3, destroyInstancepython3);
    exercise_factory(createInstanceruby, destroyInstanceruby);
    exercise_factory(createInstanceschema, destroyInstanceschema);
}

TEST(CompareImplMoreCases) {
    test_hooks::reset();
    TempDir tmp;
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(tmp.path);
    write_file(tmp.path / "data.in", "1\n");
    auto a = tmp.path / "a.out";
    auto b = tmp.path / "b.out";
    write_file(a, "1\n");
    write_file(b, "1\n");
    choose = 0;
    CompareImpl cmp;
    cmp.setDebug(1);
    cmp.setFullDiff(0);
    EXPECT_EQ(cmp.compare(a.string().c_str(), b.string().c_str()), ACCEPT);
    write_file(b, "2\n");
    cmp.setFullDiff(1);
    EXPECT_EQ(cmp.compare(a.string().c_str(), b.string().c_str()), WRONG_ANSWER);
    EXPECT_EQ(cmp.compare("missing.out", b.string().c_str()), RUNTIME_ERROR);
    auto ans = tmp.path / "ans.txt";
    auto user = tmp.path / "user.txt";
    write_file(ans, "1 2");
    write_file(user, "1    2");
    EXPECT_TRUE(check_valid_presentation_error(ans.string().c_str(), user.string().c_str()));
    std::filesystem::current_path(old_cwd);
}

TEST(CompareBaseAccessors) {
    CompareImpl cmp;
    cmp.setDebug(1);
    cmp.setFullDiff(1);
    EXPECT_EQ(cmp.getDebug(), 1);
    EXPECT_EQ(cmp.getFullDiff(), 1);
}

TEST(CompareImplPresentationError) {
    test_hooks::reset();
    TempDir tmp;
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(tmp.path);
    auto ans = tmp.path / "ans.out";
    auto user = tmp.path / "user.out";
    write_file(ans, "1 2\n");
    write_file(user, "1    2\n");
    CompareImpl cmp;
    cmp.setDebug(0);
    cmp.setFullDiff(0);
    choose = 0;
    int res = cmp.compare(ans.string().c_str(), user.string().c_str());
    EXPECT_TRUE(res == PRESENTATION_ERROR || res == ACCEPT);
    std::filesystem::current_path(old_cwd);
}

TEST(CompareImplFullDiffOutput) {
    test_hooks::reset();
    TempDir tmp;
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(tmp.path);
    write_file(tmp.path / "data.in", "1\n");
    auto ans = tmp.path / "ans.out";
    auto user = tmp.path / "user.out";
    write_file(ans, "1\n");
    write_file(user, "2\n");
    CompareImpl cmp;
    cmp.setDebug(0);
    cmp.setFullDiff(1);
    choose = 0;
    EXPECT_EQ(cmp.compare(ans.string().c_str(), user.string().c_str()), WRONG_ANSWER);
    std::filesystem::current_path(old_cwd);
}

TEST(CompareImplHelperFunctions) {
    EXPECT_TRUE(is_not_character('\n'));
    EXPECT_TRUE(!is_not_character('A'));
    EXPECT_TRUE(is_number("123"));
    EXPECT_TRUE(!is_number("12a"));

    TempDir tmp;
    auto file = tmp.path / "space.txt";
    write_file(file, " \n\tX");
    FILE *pf = fopen(file.string().c_str(), "re");
    int c = fgetc(pf);
    int ret = 0;
    move_to_next_nonspace_character(c, pf, ret);
    EXPECT_EQ(c, 'X');
    fclose(pf);

    auto file2 = tmp.path / "space2.txt";
    write_file(file2, " \nY");
    FILE *f1 = fopen(file.string().c_str(), "re");
    FILE *f2 = fopen(file2.string().c_str(), "re");
    int c1 = fgetc(f1);
    int c2 = fgetc(f2);
    find_next_nonspace(c1, c2, f1, f2, ret, 1);
    EXPECT_TRUE(c1 == 'X' || c2 == 'Y');
    fclose(f1);
    fclose(f2);
}

TEST(CompareImplEOFWhitespace) {
    test_hooks::reset();
    TempDir tmp;
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(tmp.path);
    auto ans = tmp.path / "ans.out";
    auto user = tmp.path / "user.out";
    write_file(ans, "1");
    write_file(user, "1 \n");
    CompareImpl cmp;
    cmp.setDebug(0);
    cmp.setFullDiff(0);
    choose = 0;
    int res = cmp.compare(ans.string().c_str(), user.string().c_str());
    EXPECT_TRUE(res == PRESENTATION_ERROR || res == ACCEPT);
    std::filesystem::current_path(old_cwd);
}

TEST(CompareImplDebugMismatch) {
    test_hooks::reset();
    TempDir tmp;
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(tmp.path);
    auto ans = tmp.path / "ans.out";
    auto user = tmp.path / "user.out";
    write_file(ans, "1\n");
    write_file(user, "2\n");
    CompareImpl cmp;
    cmp.setDebug(1);
    cmp.setFullDiff(0);
    choose = 0;
    EXPECT_EQ(cmp.compare(ans.string().c_str(), user.string().c_str()), WRONG_ANSWER);
    std::filesystem::current_path(old_cwd);
}

TEST(CompareImplCheckValidMismatch) {
    TempDir tmp;
    auto ans = tmp.path / "ans.txt";
    auto user = tmp.path / "user.txt";
    write_file(ans, "1 2");
    write_file(user, "1 3");
    EXPECT_TRUE(!check_valid_presentation_error(ans.string().c_str(), user.string().c_str()));
}

TEST(CompareImplCheckValidNonNumber) {
    TempDir tmp;
    auto ans = tmp.path / "ans.txt";
    auto user = tmp.path / "user.txt";
    write_file(ans, "x 2");
    write_file(user, "1 2");
    EXPECT_TRUE(check_valid_presentation_error(ans.string().c_str(), user.string().c_str()));
}

TEST(CompareImplFindNextNonspaceEOFBranch) {
    TempDir tmp;
    auto f1_path = tmp.path / "f1.txt";
    auto f2_path = tmp.path / "f2.txt";
    write_file(f1_path, " \nA");
    write_file(f2_path, "");
    FILE *f1 = fopen(f1_path.string().c_str(), "re");
    FILE *f2 = fopen(f2_path.string().c_str(), "re");
    int c1 = fgetc(f1);
    int c2 = fgetc(f2);
    int ret = 0;
    find_next_nonspace(c1, c2, f1, f2, ret, 0);
    EXPECT_EQ(c1, 'A');
    fclose(f1);
    fclose(f2);
}

TEST(CompareImplFindNextNonspaceIgnoreEsol) {
    TempDir tmp;
    auto f1_path = tmp.path / "f1.txt";
    auto f2_path = tmp.path / "f2.txt";
    write_file(f1_path, "A");
    write_file(f2_path, "B");
    FILE *f1 = fopen(f1_path.string().c_str(), "re");
    FILE *f2 = fopen(f2_path.string().c_str(), "re");
    int ret = 0;
    int c1 = '\r';
    int c2 = '\n';
    find_next_nonspace(c1, c2, f1, f2, ret, 0);
    fclose(f1);
    fclose(f2);

    f1 = fopen(f1_path.string().c_str(), "re");
    f2 = fopen(f2_path.string().c_str(), "re");
    c1 = '\n';
    c2 = '\r';
    find_next_nonspace(c1, c2, f1, f2, ret, 0);
    fclose(f1);
    fclose(f2);
}

TEST(CompareImplDebugPromptBreaks) {
    test_hooks::reset();
    TempDir tmp;
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(tmp.path);
    auto ans = tmp.path / "ans.out";
    auto user = tmp.path / "user.out";
    write_file(ans, "1\n");
    write_file(user, "1\n");

    choose = 1;
    std::istringstream in1("0\n");
    auto *old_buf = std::cin.rdbuf(in1.rdbuf());
    compare_zoj(ans.string().c_str(), user.string().c_str(), 1, 0);
    std::cin.rdbuf(old_buf);

    choose = 1;
    std::istringstream in2("1\n0\n");
    old_buf = std::cin.rdbuf(in2.rdbuf());
    compare_zoj(ans.string().c_str(), user.string().c_str(), 1, 0);
    std::cin.rdbuf(old_buf);
    std::filesystem::current_path(old_cwd);
}

TEST(CompareImplPresentationWhitespaceBranches) {
    test_hooks::reset();
    TempDir tmp;
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(tmp.path);
    auto ans = tmp.path / "ans.out";
    auto user = tmp.path / "user.out";
    write_file(ans, "1\n");
    write_file(user, "1 \n");
    int res = compare_zoj(ans.string().c_str(), user.string().c_str(), 0, 0);
    EXPECT_TRUE(res == PRESENTATION_ERROR || res == ACCEPT);
    std::filesystem::current_path(old_cwd);
}

TEST(CompareImplEmptyFiles) {
    test_hooks::reset();
    TempDir tmp;
    auto ans = tmp.path / "ans.out";
    auto user = tmp.path / "user.out";
    write_file(ans, "");
    write_file(user, "");
    EXPECT_EQ(compare_zoj(ans.string().c_str(), user.string().c_str(), 0, 0), ACCEPT);
}

TEST(CompareImplOneEOF) {
    test_hooks::reset();
    TempDir tmp;
    auto ans = tmp.path / "ans.out";
    auto user = tmp.path / "user.out";
    write_file(ans, "1");
    write_file(user, "");
    EXPECT_EQ(compare_zoj(ans.string().c_str(), user.string().c_str(), 0, 0), WRONG_ANSWER);
}

TEST(MySQLAutoPointerEdgeCases) {
    test_hooks::reset();
    MySQLAutoPointer ptr;
    EXPECT_TRUE(!ptr.setHostName(""));
    EXPECT_TRUE(!ptr.setUserName(""));
    EXPECT_TRUE(!ptr.setPassword(""));
    EXPECT_TRUE(!ptr.setDBName(""));
    EXPECT_TRUE(!ptr.setPort(0));
    test_hooks::state().mysql_start_ok = false;
    EXPECT_TRUE(!ptr.start());
    test_hooks::reset();
    MySQLAutoPointer ptr2;
    EXPECT_TRUE(ptr2.setHostName("h"));
    EXPECT_TRUE(ptr2.setUserName("u"));
    EXPECT_TRUE(ptr2.setPassword("p"));
    EXPECT_TRUE(ptr2.setDBName("d"));
    EXPECT_TRUE(ptr2.setPort(1));
    EXPECT_TRUE(ptr2.start());
    test_hooks::state().mysql_query_results.push_back(1);
    test_hooks::state().mysql_query_results.push_back(0);
    test_hooks::state().mysql_error = "gone away";
    EXPECT_EQ(ptr2.query("select 1"), 0);
    std::string sql = "select 1";
    EXPECT_EQ(ptr2.query(sql, sql.size()), 0);
}

TEST(MySQLAutoPointerDebugFailures) {
    test_hooks::reset();
    MySQLAutoPointer ptr;
    ptr.setHostName("h");
    ptr.setUserName("u");
    ptr.setPassword("p");
    ptr.setDBName("d");
    ptr.setPort(1);
    ptr.setDebugMode(true);
    test_hooks::state().mysql_start_ok = false;
    test_hooks::state().mysql_error = "connect error";
    EXPECT_TRUE(!ptr.start());

    test_hooks::reset();
    MySQLAutoPointer ptr2;
    ptr2.setHostName("h");
    ptr2.setUserName("u");
    ptr2.setPassword("p");
    ptr2.setDBName("d");
    ptr2.setPort(1);
    ptr2.setDebugMode(true);
    test_hooks::state().mysql_query_results.push_back(1);
    test_hooks::state().mysql_error = "query error";
    EXPECT_TRUE(!ptr2.start());
}

TEST(MySQLAutoPointerAutoStart) {
    test_hooks::reset();
    MySQLAutoPointer ptr;
    ptr.setHostName("h");
    ptr.setUserName("u");
    ptr.setPassword("p");
    ptr.setDBName("d");
    ptr.setPort(1);
    EXPECT_EQ(ptr.query("select 1"), 0);
    EXPECT_TRUE(ptr.isConnected());
}

TEST(MySQLSubmissionInfoManagerEdgeCases) {
    test_hooks::reset();
    test_hooks::push_mysql_result({});
    test_hooks::push_mysql_result({{"0", "32", "1"}});
    MySQLSubmissionInfoManager mgr;
    mgr.setHostName("h").setUserName("u").setPassword("p").setDBName("d").setPort(1).start();
    TempDir tmp;
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(tmp.path);
    mgr.getCustomInput(1, const_cast<char *>(tmp.path.string().c_str()));
    double tl = 0;
    int ml = 0;
    int spj = 0;
    mgr.getProblemInfo(1, tl, ml, spj);
    EXPECT_EQ(tl, 1);
    EXPECT_EQ(ml, 32);
    EXPECT_EQ(spj, 1);
    std::filesystem::current_path(old_cwd);
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
    print_runtimeerror("oops");
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

TEST(WSJudgedDebug) {
    test_hooks::reset();
    const char *argv[] = {"wsjudged", "1", "2", "/tmp", "DEBUG"};
    EXPECT_EQ(wsjudged_main(5, argv), 0);
    EXPECT_EQ(test_hooks::state().last_exec_path, "/usr/bin/judge_client");
}


static void write_basic_config(const std::filesystem::path &root,
                               bool enable_parallel, bool all_test_mode) {
    std::ostringstream ss;
    ss << "{"
       << "\"hostname\":\"localhost\","
       << "\"username\":\"u\","
       << "\"password\":\"p\","
       << "\"db_name\":\"jol\","
       << "\"port\":3306,"
       << "\"java_time_bonus\":2,"
       << "\"java_memory_bonus\":0,"
       << "\"java_xms\":\"-Xms64M\","
       << "\"java_xmx\":\"-Xmx512M\","
       << "\"sim_enable\":0,"
       << "\"full_diff\":0,"
       << "\"judger_name\":\"J\","
       << "\"shm_run\":0,"
       << "\"use_max_time\":1,"
       << "\"use_ptrace\":0,"
       << "\"all_test_mode\":" << (all_test_mode ? 1 : 0) << ","
       << "\"enable_parallel\":" << (enable_parallel ? 1 : 0)
       << "}";
    write_file(root / "etc" / "config.json", ss.str());
    write_file(root / "etc" / "compile.json", "{\"0\":[\"cc\",\"Main.c\"]}");
    write_file(root / "etc" / "language.json", "{\"0\":\"fake\"}");
}

static void write_submission(const std::filesystem::path &root,
                             const std::string &judger_id,
                             int problem_id, bool test_run) {
    std::ostringstream ss;
    ss << "{"
       << "\"language\":0,"
       << "\"user_id\":\"u\","
       << "\"problem_id\":" << problem_id << ","
       << "\"spj\":false,"
       << "\"memory_limit\":64,"
       << "\"time_limit\":1,"
       << "\"source\":\"code\","
       << "\"solution_id\":1";
    if (test_run) {
        ss << ",\"test_run\":true,\"custom_input\":\"1 2\"";
    }
    ss << "}";
    write_file(root / "submission" / (judger_id + ".json"), ss.str());
}

static void prepare_case_files(const std::filesystem::path &root, int problem_id) {
    auto data_dir = root / "data" / std::to_string(problem_id);
    write_file(data_dir / "1.in", "");
    write_file(data_dir / "1.out", "");
}

static void prepare_run_dir(const std::filesystem::path &root, int runner_id,
                            bool parallel) {
    auto run_dir = root / ("run" + std::to_string(runner_id));
    std::filesystem::create_directories(run_dir);
    if (parallel) {
        write_file(run_dir / "user0.out", "");
    } else {
        write_file(run_dir / "user.out", "");
    }
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
    expect_exit([&]() { compile(0, work_str.data()); }, 0);

    test_hooks::reset();
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    std::strcpy(oj_home, tmp.path.string().c_str());
    test_hooks::state().compile_result = 0;
    test_hooks::state().fork_results.push_back(123);
    EXPECT_EQ(compile(0, work_str.data()), 0);
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
    expect_exit([&]() { compile(0, work_str.data()); }, 0);
    test_hooks::reset();
    languageNameReader.loadJSON("{\"0\":\"fake\"}");
    std::strcpy(oj_home, tmp.path.string().c_str());
    test_hooks::state().compile_result = 0;
    EXPECT_EQ(compile(0, work_str.data()), 0);
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
    expect_exit([&]() { run_solution(lang, work.data(), tl, used, mem); }, 0);
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
    expect_exit([&]() { run_solution_parallel(lang, work.data(), tl, used, mem, 1); }, 0);
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
    expect_exit([&]() { run_solution(lang, work.data(), tl, used, mem); }, 0);
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
    expect_exit([&]() { run_solution(lang, work.data(), tl, used, mem); }, 0);
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

    int ac = ACCEPT;
    int top = STD_MB * 2;
    test_hooks::state().wait4_status = 0;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()));
    EXPECT_EQ(ac, MEMORY_LIMIT_EXCEEDED);

    ac = ACCEPT;
    top = 0;
    write_file(tmp.path / "error.out", "Killed");
    write_file(std::filesystem::path("error.out"), "Killed");
    test_hooks::state().wait4_status = 1;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, 64,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()));
    EXPECT_TRUE(ac == ACCEPT || ac == RUNTIME_ERROR);

    ac = ACCEPT;
    top = 0;
    write_file(user_path, std::string(3000, 'a'));
    write_file(out_path, "1");
    write_file(tmp.path / "error.out", "");
    test_hooks::state().wait4_status = 1;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, 64,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()));
    EXPECT_EQ(ac, OUTPUT_LIMIT_EXCEEDED);

    ac = ACCEPT;
    top = 0;
    write_file(user_path, "1");
    test_hooks::state().wait4_status = (SIGXCPU << 8) | 1;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, 64,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()));
    EXPECT_TRUE(ac != ACCEPT);

    ac = ACCEPT;
    top = 0;
    test_hooks::state().wait4_status = SIGXCPU;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, 64,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()));
    EXPECT_TRUE(ac != ACCEPT);
    std::filesystem::current_path(old_cwd);
}

TEST(JudgeClientWatchSolutionKilledDebug) {
    test_hooks::reset();
    TempDir tmp;
    std::string root = tmp.path.string();
    std::strcpy(oj_home, root.c_str());
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
    test_hooks::state().wait4_status = 1;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()));
    DEBUG = 0;
    ALL_TEST_MODE = 1;
    std::filesystem::current_path(old_cwd);
}

TEST(JudgeClientWatchSolutionExitcodeDebug) {
    test_hooks::reset();
    TempDir tmp;
    std::string root = tmp.path.string();
    std::strcpy(oj_home, root.c_str());
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
    test_hooks::state().wait4_status = (SIGXFSZ << 8) | 1;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()));
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
    test_hooks::state().wait4_status = SIGXFSZ;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()));
    EXPECT_EQ(ac, OUTPUT_LIMIT_EXCEEDED);
    DEBUG = 0;
    std::filesystem::current_path(old_cwd);
}

TEST(JudgeClientWatchSolutionMemoryLimitDebug) {
    test_hooks::reset();
    TempDir tmp;
    std::string root = tmp.path.string();
    std::strcpy(oj_home, root.c_str());
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
    test_hooks::state().wait4_status = 1;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()));
    DEBUG = 0;
    use_ptrace = 0;
    std::filesystem::current_path(old_cwd);
}

TEST(JudgeClientWatchSolutionRuntimeErrorBranch) {
    test_hooks::reset();
    TempDir tmp;
    std::string root = tmp.path.string();
    std::strcpy(oj_home, root.c_str());
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
    test_hooks::state().wait4_status = 1;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()));
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
    test_hooks::state().wait4_status = 1;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()));
    EXPECT_EQ(ac, OUTPUT_LIMIT_EXCEEDED);
    use_ptrace = 0;
    std::filesystem::current_path(old_cwd);
}

TEST(JudgeClientWatchSolutionPtraceBranches) {
    test_hooks::reset();
    TempDir tmp;
    std::string root = tmp.path.string();
    std::strcpy(oj_home, root.c_str());
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
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()));
    EXPECT_EQ(call_counter[12], 1);

    std::fill(call_counter, call_counter + call_array_size, 0);
    record_call = 0;
    test_hooks::state().wait4_statuses.push_back(0x7f);
    test_hooks::state().wait4_statuses.push_back(0);
    ac = ACCEPT;
    watch_solution(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                   const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                   used, tl, p_id, pe, const_cast<char *>(root.c_str()));
    EXPECT_EQ(ac, RUNTIME_ERROR);
    use_ptrace = 0;
    record_call = 0;
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
    auto res = judge_solution(ac, used, tl, 0, 1, infile, outfile, userfile, code,
                              pe, 0, work.data(), top, 64, 1, 1, global);
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
    test_hooks::state().compare_result = WRONG_ANSWER;
    int ac = ACCEPT;
    double used = 0.0;
    double tl = 1.0;
    int top = 0;
    auto res = judge_solution(ac, used, tl, 0, 1, infile, outfile, userfile, code,
                              pe, 0, work.data(), top, 64, 1, 1, global);
    EXPECT_EQ(res.ACflg, WRONG_ANSWER);

    test_hooks::state().compare_result = PRESENTATION_ERROR;
    ac = ACCEPT;
    used = 0.0;
    top = 0;
    pe = 0;
    res = judge_solution(ac, used, tl, 0, 1, infile, outfile, userfile, code,
                         pe, 0, work.data(), top, 64, 1, 1, global);
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
    test_hooks::state().fork_results.push_back(1);
    test_hooks::state().pipe_seed = "0";
    int stdout_fd = dup(fileno(stdout));
    auto res = judge_solution(ac, used, tl, 1, 1, infile, outfile, userfile, code,
                              pe, 0, work.data(), top, 64, 1, 1, global);
    EXPECT_EQ(res.ACflg, ACCEPT);
    if (stdout_fd >= 0) {
        fflush(stdout);
        dup2(stdout_fd, fileno(stdout));
        close(stdout_fd);
    }
    std::filesystem::current_path(old_cwd);
}

TEST(JudgeClientRunJudgeTaskBranches) {
    test_hooks::reset();
    TempDir tmp;
    std::string root = tmp.path.string();
    std::strcpy(oj_home, root.c_str());
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
    test_hooks::state().fork_results.push_back(0);
    int stdout_fd = dup(fileno(stdout));
    int stderr_fd = dup(fileno(stderr));
    expect_exit([&]() {
        runJudgeTask(1, 0, work.data(), infilePair, ac, 0, 1, 1.0, 0.0, 64,
                     1, usercode, 0, global);
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
                                   0.0, 64, 1, usercode, 0, global);
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

TEST(JudgeClientRunParallelJudge) {
    test_hooks::reset();
    TempDir tmp;
    std::string root = tmp.path.string();
    std::strcpy(oj_home, root.c_str());
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
    test_hooks::state().compare_result = ACCEPT;
    test_hooks::state().wait4_status = 0;
    auto res = runParallelJudge(1, 0, work.data(), const_cast<char *>("code"), 1, 0, 64,
                                inFileList, ac, 0, global, submission);
    EXPECT_TRUE(res.pass_point >= 0);
}

TEST(JudgeClientWatchSolutionWithFileIdDebug) {
    test_hooks::reset();
    TempDir tmp;
    std::string root = tmp.path.string();
    std::strcpy(oj_home, root.c_str());
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
    test_hooks::state().wait4_status = 1;
    watch_solution_with_file_id(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                                const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                                used, tl, p_id, pe, const_cast<char *>(root.c_str()), 1,
                                call_counter_local);
    DEBUG = 0;
    ALL_TEST_MODE = 1;
    std::filesystem::current_path(old_cwd);
}

TEST(JudgeClientWatchSolutionWithFileIdPtraceBranch) {
    test_hooks::reset();
    TempDir tmp;
    std::string root = tmp.path.string();
    std::strcpy(oj_home, root.c_str());
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
    test_hooks::state().ptrace_syscall = 12;
    test_hooks::state().wait4_statuses.push_back(0x7f);
    test_hooks::state().wait4_statuses.push_back(0);
    watch_solution_with_file_id(1, infile, ac, 0, const_cast<char *>(user_path.c_str()),
                                const_cast<char *>(out_path.c_str()), 1, lang, top, mem,
                                used, tl, p_id, pe, const_cast<char *>(root.c_str()), 1,
                                call_counter_local);
    EXPECT_EQ(call_counter_local[12], 1);
    use_ptrace = 0;
    record_call = 0;
    std::filesystem::current_path(old_cwd);
}

TEST(JudgeClientMySQLSim) {
    test_hooks::reset();
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
    try {
        judge_client_main(argc, const_cast<char **>(argv));
        fail("expected exit");
    } catch (const test_hooks::ExitException &ex) {
        EXPECT_EQ(ex.code, 0);
    }
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
    try {
        judge_client_main(argc, const_cast<char **>(argv));
        fail("expected exit");
    } catch (const test_hooks::ExitException &ex) {
        EXPECT_EQ(ex.code, 0);
    }
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

int main() {
    int failed = 0;
    auto base_cwd = std::filesystem::current_path();
    const char *filter = std::getenv("TEST_FILTER");
    for (const auto &t : tests()) {
        if (filter && !std::strstr(t.name, filter)) {
            continue;
        }
        std::filesystem::current_path(base_cwd);
        reset_globals_for_test();
        std::cout << "[RUN] " << t.name << "\n";
        try {
            t.func();
            std::cout << "[PASS] " << t.name << "\n";
        } catch (const TestFailure &e) {
            ++failed;
            std::cout << "[FAIL] " << t.name << ": " << e.msg << "\n";
        } catch (const test_hooks::ExitException &e) {
            ++failed;
            std::cout << "[FAIL] " << t.name << ": unexpected exit " << e.code << "\n";
        } catch (const std::exception &e) {
            ++failed;
            std::cout << "[FAIL] " << t.name << ": " << e.what() << "\n";
        }
    }
    if (__gcov_flush) {
        __gcov_flush();
    }
    if (failed) {
        std::cout << failed << " test(s) failed\n";
        return 1;
    }
    return 0;
}
