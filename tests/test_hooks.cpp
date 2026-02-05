#include "test_hooks.h"

#include <cstdarg>
#include <cstring>
#include <cerrno>
#include <cstdio>
#include <string>
#include <sys/ptrace.h>
#include <unistd.h>

#include "../header/static_var.h"
#include "../external/compare/Compare.h"
#include "../external/mysql/MySQLSubmissionAdapter.h"
#include "../model/judge/language/Language.h"

extern "C" Language* createInstancec11();
extern "C" Language* createInstancecpp11();

namespace test_hooks {
static State g_state;
void clear_mysql_results();

State &state() {
    return g_state;
}

void reset() {
    if (g_state.pipe_keep_fd >= 0) {
        close(g_state.pipe_keep_fd);
        g_state.pipe_keep_fd = -1;
    }
    g_state = State();
    g_state.main_pid = getpid();
    clear_mysql_results();
}
} // namespace test_hooks

pid_t test_fork() TEST_HOOKS_THROW {
    auto &st = test_hooks::state();
    if (!st.fork_results.empty()) {
        int v = st.fork_results.front();
        st.fork_results.pop_front();
        return v;
    }
    return st.default_fork_result;
}

#ifdef UNIT_TEST
#undef fork
extern "C" pid_t fork(void) {
    return test_fork();
}

extern "C" pid_t vfork(void) {
    return test_fork();
}

extern "C" pid_t __fork(void) {
    return test_fork();
}

extern "C" pid_t __libc_fork(void) {
    return test_fork();
}

extern "C" pid_t __vfork(void) {
    return test_fork();
}
#endif

pid_t test_waitpid(pid_t pid, int *status, int options) TEST_HOOKS_THROW {
    (void)pid;
    (void)options;
    if (status) {
        *status = test_hooks::state().waitpid_status;
    }
    return 0;
}

pid_t test_wait4(pid_t pid, int *status, int options, struct rusage *rusage) TEST_HOOKS_THROW {
    (void)pid;
    (void)options;
    auto &st = test_hooks::state();
    if (status) {
        if (!st.wait4_statuses.empty()) {
            *status = st.wait4_statuses.front();
            st.wait4_statuses.pop_front();
        } else {
            *status = st.wait4_status;
        }
    }
    if (rusage) {
        std::memset(rusage, 0, sizeof(*rusage));
        rusage->ru_utime.tv_sec = st.wait4_utime_ms / 1000;
        rusage->ru_utime.tv_usec = (st.wait4_utime_ms % 1000) * 1000;
        rusage->ru_stime.tv_sec = st.wait4_stime_ms / 1000;
        rusage->ru_stime.tv_usec = (st.wait4_stime_ms % 1000) * 1000;
    }
    return 0;
}

int test_execvp(const char *file, char *const argv[]) TEST_HOOKS_THROW {
    (void)argv;
    test_hooks::state().last_exec_path = file ? file : "";
    return 0;
}

int test_execv(const char *path, char *const argv[]) TEST_HOOKS_THROW {
    (void)argv;
    test_hooks::state().last_exec_path = path ? path : "";
    return 0;
}

int test_execl(const char *path, const char *arg, ...) TEST_HOOKS_THROW {
    (void)arg;
    test_hooks::state().last_exec_path = path ? path : "";
    return 0;
}

int test_execve(const char *path, char *const argv[], char *const envp[]) TEST_HOOKS_THROW {
    (void)argv;
    (void)envp;
    test_hooks::state().last_exec_path = path ? path : "";
    return 0;
}

#ifdef UNIT_TEST
#undef execv
#undef execvp
#undef execl
 #undef execve
extern "C" int execv(const char *path, char *const argv[]) {
    return test_execv(path, argv);
}

extern "C" int execvp(const char *file, char *const argv[]) {
    return test_execvp(file, argv);
}

extern "C" int execl(const char *path, const char *arg, ...) {
    return test_execl(path, arg);
}

extern "C" int execve(const char *path, char *const argv[], char *const envp[]) {
    return test_execve(path, argv, envp);
}

extern "C" int __execv(const char *path, char *const argv[]) {
    return test_execv(path, argv);
}

extern "C" int __execvp(const char *file, char *const argv[]) {
    return test_execvp(file, argv);
}

extern "C" int __libc_execv(const char *path, char *const argv[]) {
    return test_execv(path, argv);
}

extern "C" int __libc_execvp(const char *file, char *const argv[]) {
    return test_execvp(file, argv);
}

extern "C" int __execve(const char *path, char *const argv[], char *const envp[]) {
    return test_execve(path, argv, envp);
}

extern "C" int __libc_execve(const char *path, char *const argv[], char *const envp[]) {
    return test_execve(path, argv, envp);
}
#endif

long test_ptrace(enum __ptrace_request request, ...) TEST_HOOKS_THROW {
    va_list args;
    va_start(args, request);
    pid_t pid = va_arg(args, pid_t);
    void *addr = va_arg(args, void *);
    void *data = va_arg(args, void *);
    va_end(args);
    (void)pid;
    (void)addr;
    if (request == PTRACE_GETREGS && data) {
        auto *regs = reinterpret_cast<user_regs_struct *>(data);
#if defined(__x86_64__)
        regs->orig_rax = test_hooks::state().ptrace_syscall;
#elif defined(__i386__)
        regs->orig_eax = test_hooks::state().ptrace_syscall;
#endif
    }
    return 0;
}

int test_setrlimit(int resource, const struct rlimit *rlim) TEST_HOOKS_THROW {
    (void)resource;
    (void)rlim;
    return 0;
}

unsigned int test_alarm(unsigned int seconds) TEST_HOOKS_THROW {
    return seconds;
}

int test_chroot(const char *path) TEST_HOOKS_THROW {
    (void)path;
    return 0;
}

int test_setgid(gid_t gid) TEST_HOOKS_THROW {
    (void)gid;
    return 0;
}

int test_setuid(uid_t uid) TEST_HOOKS_THROW {
    (void)uid;
    return 0;
}

int test_setresuid(uid_t ruid, uid_t euid, uid_t suid) TEST_HOOKS_THROW {
    (void)ruid;
    (void)euid;
    (void)suid;
    return 0;
}

int test_system(const char *command) TEST_HOOKS_THROW {
    if (command) {
        test_hooks::state().last_exec_path = command;
    }
    return test_hooks::state().system_result;
}

void *test_dlopen(const char *filename, int flag) TEST_HOOKS_THROW {
    (void)flag;
    auto &st = test_hooks::state();
    st.last_dlopen_path = filename ? filename : "";
    ++st.dlopen_calls;
    return reinterpret_cast<void *>(0x1);
}

namespace {
class FakeLanguage : public Language {
public:
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

    int getCompileResult(int status) override {
        (void)status;
        return test_hooks::state().compile_result;
    }
};

class FakeCompare : public Compare::Compare {
public:
    int compare(const char *stdAnswerFile, const char *userOutputFile) override {
        (void)stdAnswerFile;
        (void)userOutputFile;
        int result = test_hooks::state().compare_result;
        if (result == WRONG_ANSWER || result == PRESENTATION_ERROR) {
            FILE *fp = fopen("diff.out", "a+");
            if (fp) {
                fputs("diff", fp);
                fclose(fp);
            }
        }
        if (result == RUNTIME_ERROR) {
            FILE *fp = fopen("error.out", "a+");
            if (fp) {
                fputs("Runtime Error", fp);
                fclose(fp);
            }
        }
        return result;
    }
};

class FakeMySQLAdapter : public MySQLSubmissionAdapter {
public:
    void getCustomInput(int solutionId, char *work_dir) override {
        (void)solutionId;
        FILE *fp = fopen((std::string(work_dir) + "/data.in").c_str(), "w");
        if (fp) {
            fprintf(fp, "%s", test_hooks::state().mysql_custom_input.c_str());
            fclose(fp);
        }
    }

    void getProblemInfo(int p_id, double &time_lmt, int &mem_lmt, int &isspj) override {
        (void)p_id;
        time_lmt = test_hooks::state().mysql_time_limit;
        mem_lmt = test_hooks::state().mysql_memory_limit;
        isspj = test_hooks::state().mysql_spj;
    }

    void getSolution(int solution_id, char *work_dir, int lang, char *usercode,
                     const char *suffix, int DEBUG) override {
        (void)solution_id;
        (void)lang;
        (void)DEBUG;
        std::strcpy(usercode, test_hooks::state().mysql_source.c_str());
        std::string src = std::string(work_dir) + "/Main." + suffix;
        FILE *fp = fopen(src.c_str(), "w");
        if (fp) {
            fprintf(fp, "%s", test_hooks::state().mysql_source.c_str());
            fclose(fp);
        }
    }

    void getSolutionInfo(int solutionId, int &p_id, char *userId, int &lang) override {
        (void)solutionId;
        p_id = test_hooks::state().mysql_problem_id;
        std::strcpy(userId, test_hooks::state().mysql_user_id.c_str());
        lang = test_hooks::state().mysql_language;
    }

    MySQLSubmissionAdapter &setPort(int port) override {
        (void)port;
        return *this;
    }

    MySQLSubmissionAdapter &setDBName(const std::string db) override {
        (void)db;
        return *this;
    }

    MySQLSubmissionAdapter &setUserName(const std::string &user_name) override {
        (void)user_name;
        return *this;
    }

    MySQLSubmissionAdapter &setPassword(const std::string &password) override {
        (void)password;
        return *this;
    }

    MySQLSubmissionAdapter &setHostName(const std::string host_name) override {
        (void)host_name;
        return *this;
    }

    bool isConnected() override {
        return true;
    }

    bool start() override {
        return test_hooks::state().mysql_start_ok;
    }
};

FakeLanguage *create_language() {
    return new FakeLanguage();
}

FakeCompare *create_compare() {
    return new FakeCompare();
}

FakeMySQLAdapter *create_mysql() {
    return new FakeMySQLAdapter();
}

void destroy_any(void *ptr) {
    (void)ptr;
}
} // namespace

void *test_dlsym(void *handle, const char *symbol) TEST_HOOKS_THROW {
    (void)handle;
    ++test_hooks::state().dlsym_calls;
    std::string sym = symbol ? symbol : "";
    const auto &path = test_hooks::state().last_dlopen_path;
    if (sym.find("destroy") == 0) {
        return reinterpret_cast<void *>(&destroy_any);
    }
    if (sym.find("createInstance") != std::string::npos) {
        if (path.find("libcompare") != std::string::npos) {
            return reinterpret_cast<void *>(&create_compare);
        }
        if (path.find("libmysql") != std::string::npos) {
            return reinterpret_cast<void *>(&create_mysql);
        }
        if (path.find("libc11") != std::string::npos) {
            return reinterpret_cast<void *>(&createInstancec11);
        }
        if (path.find("libcpp11") != std::string::npos) {
            return reinterpret_cast<void *>(&createInstancecpp11);
        }
        return reinterpret_cast<void *>(&create_language);
    }
    return nullptr;
}

char *test_dlerror(void) TEST_HOOKS_THROW {
    return nullptr;
}

#ifdef UNIT_TEST
#undef dlopen
#undef dlsym
#undef dlerror
extern "C" void *dlopen(const char *filename, int flag) {
    return test_dlopen(filename, flag);
}

extern "C" void *dlsym(void *handle, const char *symbol) {
    return test_dlsym(handle, symbol);
}

extern "C" char *dlerror(void) {
    return test_dlerror();
}
#endif

void test_exit(int code) {
    auto &st = test_hooks::state();
    if (st.main_pid != 0 && getpid() != st.main_pid) {
        _Exit(code);
    }
    test_hooks::state().last_exit_code = code;
    if (test_hooks::state().exit_throws) {
        throw test_hooks::ExitException{code};
    }
}

#ifdef UNIT_TEST
#undef pipe
extern "C" int pipe(int pipefd[2]) TEST_HOOKS_THROW;
#endif

int test_pipe(int pipefd[2]) TEST_HOOKS_THROW {
    if (test_hooks::state().pipe_fail) {
        if (test_hooks::state().pipe_errno) {
            errno = test_hooks::state().pipe_errno;
        }
        test_hooks::state().pipe_fail = false;
        return -1;
    }
    int ret = ::pipe(pipefd);
    if (ret == 0 && test_hooks::state().pipe_keep_open &&
        test_hooks::state().pipe_keep_fd < 0) {
        test_hooks::state().pipe_keep_fd = ::dup(pipefd[0]);
    }
    if (ret == 0 && !test_hooks::state().pipe_seed.empty()) {
        const std::string &seed = test_hooks::state().pipe_seed;
        ::write(pipefd[1], seed.data(), seed.size());
        char nul = '\0';
        ::write(pipefd[1], &nul, 1);
    }
    return ret;
}

int test_seccomp_load(scmp_filter_ctx ctx) TEST_HOOKS_THROW {
    (void)ctx;
    return 0;
}

// Fake mysql API implementation for tests.
#include "fake_mysql/mysql/mysql.h"
#include <vector>

struct MYSQL {
    int last_error;
};

struct MYSQL_RES {
    std::vector<std::vector<std::string>> rows;
    size_t idx = 0;
    std::vector<std::vector<char *>> row_ptrs;
};

static std::deque<MYSQL_RES *> g_results;

extern "C" {
MYSQL *mysql_init(MYSQL *mysql) {
    if (mysql) {
        mysql->last_error = 0;
        return mysql;
    }
    auto *m = new MYSQL();
    m->last_error = 0;
    return m;
}

int mysql_options(MYSQL *mysql, int option, const void *arg) {
    (void)mysql;
    (void)option;
    (void)arg;
    return 0;
}

MYSQL *mysql_real_connect(MYSQL *mysql, const char *host, const char *user,
                          const char *passwd, const char *db, unsigned int port,
                          const char *unix_socket, unsigned long clientflag) {
    (void)host;
    (void)user;
    (void)passwd;
    (void)db;
    (void)port;
    (void)unix_socket;
    (void)clientflag;
    if (!test_hooks::state().mysql_start_ok) {
        return nullptr;
    }
    return mysql;
}

int mysql_real_query(MYSQL *mysql, const char *stmt_str, unsigned long length) {
    (void)mysql;
    (void)stmt_str;
    (void)length;
    auto &st = test_hooks::state();
    if (!st.mysql_query_results.empty()) {
        int ret = st.mysql_query_results.front();
        st.mysql_query_results.pop_front();
        return ret;
    }
    return 0;
}

MYSQL_RES *mysql_store_result(MYSQL *mysql) {
    (void)mysql;
    if (g_results.empty()) {
        return nullptr;
    }
    auto *res = g_results.front();
    g_results.pop_front();
    return res;
}

MYSQL_ROW mysql_fetch_row(MYSQL_RES *res) {
    if (!res || res->idx >= res->rows.size()) {
        return nullptr;
    }
    if (res->row_ptrs.empty()) {
        res->row_ptrs.resize(res->rows.size());
        for (size_t i = 0; i < res->rows.size(); ++i) {
            res->row_ptrs[i].resize(res->rows[i].size());
            for (size_t j = 0; j < res->rows[i].size(); ++j) {
                res->row_ptrs[i][j] = const_cast<char *>(res->rows[i][j].c_str());
            }
        }
    }
    return res->row_ptrs[res->idx++].data();
}

void mysql_free_result(MYSQL_RES *res) {
    delete res;
}

const char *mysql_error(MYSQL *mysql) {
    (void)mysql;
    if (!test_hooks::state().mysql_error.empty()) {
        return test_hooks::state().mysql_error.c_str();
    }
    return "";
}

void mysql_close(MYSQL *mysql) {
    delete mysql;
}
} // extern "C"

void test_hooks::push_mysql_result(const std::vector<std::vector<std::string>> &rows) {
    auto *res = new MYSQL_RES();
    res->rows = rows;
    g_results.push_back(res);
}

void test_hooks::clear_mysql_results() {
    while (!g_results.empty()) {
        delete g_results.front();
        g_results.pop_front();
    }
}
