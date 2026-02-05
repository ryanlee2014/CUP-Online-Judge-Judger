#pragma once

#include "test_registry.h"
#include "test_hooks.h"

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <ucontext.h>
#include <unistd.h>
#include <utility>
#include <vector>

#include "../external/compare/CompareImpl.h"
#include "../external/mysql/MySQLAutoPointer.h"
#include "../external/mysql/MySQLSubmissionInfoManager.h"
#include "../header/static_var.h"
#include "../judge_client_compile_helpers.h"
#include "../judge_client_context.h"
#include "../judge_client_context_helpers.h"
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
extern "C" void __gcov_exit(void) __attribute__((weak));

int judge_client_main(int argc, char **argv);
int wsjudged_main(int argc, const char **argv);
bool check_valid_presentation_error(const char *ansfile, const char *userfile);
void find_next_nonspace(int &c1, int &c2, FILE *&f1, FILE *&f2, int &ret, int DEBUG);
int compile(int lang, char *work_dir, const JudgeEnv &env,
            const JudgeConfigSnapshot &config, bool debug);
FILE *read_cmd_output(const char *fmt, ...);
void run_solution_parallel(int &lang, char *work_dir, double &time_lmt, double &usedtime,
                           int &mem_lmt, int fileId, const JudgeConfigSnapshot &config);
void run_solution(int &lang, char *work_dir, double &time_lmt, double &usedtime,
                  int &mem_lmt, const JudgeConfigSnapshot &config);
void run_solution(int &lang, char *work_dir, const double &time_lmt, const double &usedtime,
                  const int &mem_lmt, const JudgeConfigSnapshot &config);
void run_solution_parallel(int &lang, char *work_dir, double &time_lmt, double &usedtime,
                           int &mem_lmt, int fileId, const JudgeConfigSnapshot &config,
                           const LanguageFactory &language_factory);
void run_solution(int &lang, char *work_dir, double &time_lmt, double &usedtime,
                  int &mem_lmt, const JudgeConfigSnapshot &config, const LanguageFactory &language_factory);
void run_solution(int &lang, char *work_dir, const double &time_lmt, const double &usedtime,
                  const int &mem_lmt, const JudgeConfigSnapshot &config, const LanguageFactory &language_factory);
void watch_solution(pid_t pidApp, char *infile, int &ACflg, int isspj,
                    char *userfile, char *outfile, int solution_id, int lang,
                    int &topmemory, int mem_lmt, double &usedtime, double time_lmt, int &p_id,
                    int &PEflg, char *work_dir, const JudgeConfigSnapshot &config,
                    const JudgeEnv &env, bool record_syscall, bool debug_enabled);
void watch_solution_with_file_id(pid_t pidApp, char *infile, int &ACflg, int isspj,
                                 char *userfile, char *outfile, int solution_id, int lang,
                                 int &topmemory, int mem_lmt, double &usedtime, double time_lmt, int &p_id,
                                 int &PEflg, char *work_dir, int file_id, int *call_counter,
                                 const JudgeConfigSnapshot &config, const JudgeEnv &env,
                                 bool record_syscall, bool debug_enabled);
JudgeResult runJudgeTask(int runner_id, int language, char *work_dir,
                         const std::pair<std::string, int> &infilePair, int ACflg, int SPECIAL_JUDGE,
                         int solution_id, double timeLimit, double usedtime, int memoryLimit,
                         int problemId, char *usercode, int num_of_test, std::string &global_work_dir,
                         const JudgeConfigSnapshot &config, const JudgeEnv &env,
                         bool record_syscall, bool debug_enabled);
JudgeResult runJudgeTask(int runner_id, int language, char *work_dir,
                         const std::pair<std::string, int> &infilePair, int ACflg, int SPECIAL_JUDGE,
                         int solution_id, double timeLimit, double usedtime, int memoryLimit,
                         int problemId, char *usercode, int num_of_test, std::string &global_work_dir,
                         const JudgeConfigSnapshot &config, const JudgeEnv &env,
                         bool record_syscall, bool debug_enabled, const int *syscall_template,
                         const LanguageFactory &language_factory);
JudgeResult runJudgeTask(int runner_id, int language, char *work_dir,
                         const std::pair<std::string, int> &infilePair, int ACflg, int SPECIAL_JUDGE,
                         int solution_id, double timeLimit, double usedtime, int memoryLimit,
                         int problemId, char *usercode, int num_of_test, std::string &global_work_dir,
                         const JudgeConfigSnapshot &config, const JudgeEnv &env,
                         bool record_syscall, bool debug_enabled, const int *syscall_template,
                         const LanguageFactory &language_factory, const CompareFactory &compare_factory);
JudgeSeriesResult runParallelJudge(int runner_id, int language, char *work_dir, char *usercode,
                                   int timeLimit, int usedtime, int memoryLimit,
                                   std::vector<std::pair<std::string, int>> &inFileList,
                                   int &ACflg, int SPECIAL_JUDGE, std::string &global_work_dir,
                                   SubmissionInfo &submissionInfo, const JudgeConfigSnapshot &config,
                                   const JudgeEnv &env, bool record_syscall, bool debug_enabled);
JudgeSeriesResult runParallelJudge(int runner_id, int language, char *work_dir, char *usercode,
                                   int timeLimit, int usedtime, int memoryLimit,
                                   std::vector<std::pair<std::string, int>> &inFileList,
                                   int &ACflg, int SPECIAL_JUDGE, std::string &global_work_dir,
                                   SubmissionInfo &submissionInfo, const JudgeConfigSnapshot &config,
                                   const JudgeEnv &env, bool record_syscall, bool debug_enabled,
                                   const int *syscall_template, const LanguageFactory &language_factory);
JudgeSeriesResult runParallelJudge(int runner_id, int language, char *work_dir, char *usercode,
                                   int timeLimit, int usedtime, int memoryLimit,
                                   std::vector<std::pair<std::string, int>> &inFileList,
                                   int &ACflg, int SPECIAL_JUDGE, std::string &global_work_dir,
                                   SubmissionInfo &submissionInfo, const JudgeConfigSnapshot &config,
                                   const JudgeEnv &env, bool record_syscall, bool debug_enabled,
                                   const int *syscall_template, const LanguageFactory &language_factory,
                                   const CompareFactory &compare_factory);
void init_parameters(int argc, char **argv, int &solution_id,
                     int &runner_id, std::string &judgerId);
void print_call_array();
extern int call_counter[];
extern int choose;
JudgeResult judge_solution(int &ACflg, double &usedtime, double time_lmt, int isspj,
                           int p_id, char *infile, char *outfile, char *userfile, char *usercode, int &PEflg,
                           int lang, char *work_dir, int &topmemory, int mem_lmt,
                           int solution_id, int num_of_test, std::string &global_work_dir,
                           const JudgeConfigSnapshot &config, const JudgeEnv &env, bool debug_enabled);
JudgeResult judge_solution(JudgeContext &ctx, int &ACflg, double &usedtime, double time_lmt, int isspj,
                           char *infile, char *outfile, char *userfile, char *usercode, int &PEflg,
                           char *work_dir, int &topmemory, int mem_lmt,
                           int solution_id, int num_of_test, std::string &global_work_dir);
int compare_zoj(const char *file1, const char *file2, int DEBUG, int full_diff);
bool is_not_character(int c);
bool is_number(const std::string &s);
void move_to_next_nonspace_character(int &c, FILE *&f, int &ret);
void find_next_nonspace(int &c1, int &c2, FILE *&f1, FILE *&f2, int &ret, int DEBUG);

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

inline void fail(const std::string &msg) {
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

inline void write_file(const std::filesystem::path &path, const std::string &data) {
    auto parent = path.parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent);
    }
    std::ofstream out(path, std::ios::binary);
    out << data;
}

inline std::string read_file(const std::filesystem::path &path) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

inline void normalize_file_eol(const std::filesystem::path &path) {
    std::string contents = read_file(path);
    std::string normalized;
    normalized.reserve(contents.size());
    for (char c : contents) {
        if (c != '\r') {
            normalized.push_back(c);
        }
    }
    write_file(path, normalized);
}

inline void copy_tree(const std::filesystem::path &src, const std::filesystem::path &dst) {
    std::filesystem::create_directories(dst);
    for (const auto &entry : std::filesystem::recursive_directory_iterator(src)) {
        const auto &path = entry.path();
        auto relative = std::filesystem::relative(path, src);
        auto target = dst / relative;
        if (entry.is_directory()) {
            std::filesystem::create_directories(target);
        } else {
            std::filesystem::create_directories(target.parent_path());
            std::filesystem::copy_file(path, target, std::filesystem::copy_options::overwrite_existing);
        }
    }
}

inline void expect_exit(const std::function<void()> &fn, int code) {
    struct ExitGuard {
        bool prev = false;
        ExitGuard() {
            prev = test_hooks::state().exit_throws;
            test_hooks::state().exit_throws = true;
        }
        ~ExitGuard() {
            test_hooks::state().exit_throws = prev;
        }
    } guard;
    try {
        fn();
        fail("expected exit");
    } catch (const test_hooks::ExitException &ex) {
        EXPECT_EQ(ex.code, code);
    }
}

inline void reset_globals_for_test() {
    DEBUG = 0;
    NO_RECORD = 0;
    std::memset(oj_home, 0, sizeof(oj_home));
    std::memset(host_name, 0, sizeof(host_name));
    std::memset(user_name, 0, sizeof(user_name));
    std::memset(password, 0, sizeof(password));
    std::memset(db_name, 0, sizeof(db_name));
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
    MYSQL_MODE = true;
    READ_FROM_STDIN = false;
    solution_id = 0;
    choose = 0;
}

inline bool run_seccomp() {
    const char *val = std::getenv("RUN_SECCOMP");
    if (!val || !*val) {
        return false;
    }
    return std::strcmp(val, "0") != 0;
}

inline JudgeConfigSnapshot make_config_snapshot() {
    JudgeConfigSnapshot cfg;
    cfg.java_time_bonus = javaTimeBonus;
    cfg.java_memory_bonus = java_memory_bonus;
    cfg.sim_enable = sim_enable;
    cfg.share_memory_run = SHARE_MEMORY_RUN;
    cfg.use_max_time = use_max_time;
    cfg.use_ptrace = use_ptrace;
    cfg.all_test_mode = ALL_TEST_MODE;
    cfg.enable_parallel = enable_parallel;
    cfg.java_xms = java_xms;
    cfg.java_xmx = java_xmx;
    return cfg;
}

template <typename T>
inline void exercise_seccomp_sandbox(T &lang) {
    lang.buildSeccompSandbox();
}

inline void exercise_factory(Language *(*create_fn)(), void (*destroy_fn)(Language *)) {
    Language *lang = create_fn();
    EXPECT_TRUE(lang != nullptr);
    destroy_fn(lang);
}

struct BonusLimitTester : BonusLimit {
    double time(double timeLimit, double bonus) {
        return buildBonusTimeLimit(timeLimit, bonus);
    }
    int memory(int memoryLimit, int bonus) {
        return buildBonusMemoryLimit(memoryLimit, bonus);
    }
};

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
inline void exercise_python_like(T &lang, const std::filesystem::path &root,
                                 int expected_compile_result) {
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(root);
    write_file(root / "error.out", "SyntaxError");
    write_file(root / "ce.txt", "err");
    int calls[call_array_size] = {};
    lang.initCallCounter(calls);
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
    lang.buildRuntime(work.c_str());
    if (run_seccomp()) {
        lang.buildSeccompSandbox();
    }
    lang.run(64);
    EXPECT_TRUE(!test_hooks::state().last_exec_path.empty());
    std::filesystem::current_path(old_cwd);
}

inline void write_basic_config(const std::filesystem::path &root,
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

inline void write_submission(const std::filesystem::path &root,
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

inline void prepare_case_files(const std::filesystem::path &root, int problem_id) {
    auto data_dir = root / "data" / std::to_string(problem_id);
    write_file(data_dir / "1.in", "");
    write_file(data_dir / "1.out", "");
}

inline void prepare_run_dir(const std::filesystem::path &root, int runner_id,
                            bool parallel) {
    auto run_dir = root / ("run" + std::to_string(runner_id));
    std::filesystem::create_directories(run_dir);
    if (parallel) {
        write_file(run_dir / "user0.out", "");
    } else {
        write_file(run_dir / "user.out", "");
    }
}
