#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <csignal>
#include <ucontext.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/socket.h>

#include "../external/compare/CompareImpl.h"
#include "../header/static_var.h"
#include "../library/judge_lib.h"
#include "../model/judge/language/common/seccomp_helper.h"
#include <seccomp.h>
#ifdef __i386
#include "../model/judge/language/syscall/c11/syscall32.h"
#undef JUDGE_LIBRARY_SYSCALL32_H
#include "../model/judge/language/syscall/python3/syscall32.h"
#else
#include "../model/judge/language/syscall/c11/syscall64.h"
#undef JUDGE_LIBRARY_SYSCALL64_H
#include "../model/judge/language/syscall/python3/syscall64.h"
#endif

int compare_zoj(const char *file1, const char *file2, int DEBUG, int full_diff);

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

static std::filesystem::path make_temp_dir() {
    auto base = std::filesystem::temp_directory_path();
    auto unique = std::to_string(
        std::chrono::steady_clock::now().time_since_epoch().count());
    auto path = base / ("cupjudge_real_" + unique);
    std::filesystem::create_directories(path);
    return path;
}

static int run_command(const std::string &cmd) {
    return std::system(cmd.c_str());
}

static int run_basic_end_to_end(const std::filesystem::path &root) {
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(root);
    write_file("Main.c",
               "#include <stdio.h>\n"
               "int main(){int a,b; if(scanf(\"%d%d\",&a,&b)!=2) return 1; printf(\"%d\\n\",a+b); return 0;}\n");
    if (run_command("cc -std=c11 -O2 -o main Main.c") != 0) {
        std::cerr << "compile failed" << std::endl;
        std::filesystem::current_path(old_cwd);
        return 1;
    }
    write_file("data.in", "1 2\n");
    write_file("data.out", "3\n");
    if (run_command("./main < data.in > user.out") != 0) {
        std::cerr << "run failed" << std::endl;
        std::filesystem::current_path(old_cwd);
        return 2;
    }
    int res = compare_zoj("data.out", "user.out", 0, 0);
    if (res != ACCEPT) {
        std::cerr << "compare failed: " << res << std::endl;
        std::cerr << "data.out=" << read_file("data.out") << std::endl;
        std::cerr << "user.out=" << read_file("user.out") << std::endl;
    }
    std::filesystem::current_path(old_cwd);
    return res == ACCEPT ? 0 : 3;
}

static int run_fixture_end_to_end(const std::filesystem::path &root) {
    auto fixture_root = std::filesystem::path(__FILE__).parent_path()
                        / "fixtures" / "real_project";
    copy_tree(fixture_root, root);
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(root);
    write_file("Main.c",
               "#include <stdio.h>\n"
               "int main(){int a,b; if(scanf(\"%d%d\",&a,&b)!=2) return 1; printf(\"%d\\n\",a+b); return 0;}\n");
    if (run_command("cc -std=c11 -O2 -o main Main.c") != 0) {
        std::cerr << "compile failed (fixture)" << std::endl;
        std::filesystem::current_path(old_cwd);
        return 4;
    }
    auto data_in = root / "data" / "1001" / "1.in";
    auto data_out = root / "data" / "1001" / "1.out";
    if (run_command(std::string("./main < ") + data_in.string() + " > user.out") != 0) {
        std::cerr << "run failed (fixture)" << std::endl;
        std::filesystem::current_path(old_cwd);
        return 5;
    }
    int res = compare_zoj(data_out.string().c_str(), "user.out", 0, 0);
    if (res != ACCEPT) {
        std::cerr << "fixture compare failed: " << res << std::endl;
        std::cerr << "data.out=" << read_file(data_out) << std::endl;
        std::cerr << "user.out=" << read_file("user.out") << std::endl;
    }
    std::filesystem::current_path(old_cwd);
    return res == ACCEPT ? 0 : 6;
}

static int run_illegal_syscall_case() {
    pid_t pid = fork();
    if (pid == 0) {
        siginfo_t info{};
        ucontext_t ctx{};
#ifdef SYS_SECCOMP
        info.si_code = SYS_SECCOMP;
#else
        info.si_code = 1;
#endif
#if defined(__x86_64__)
        ctx.uc_mcontext.gregs[REG_RAX] = 100000;
#elif defined(__i386__)
        ctx.uc_mcontext.gregs[REG_EAX] = 100000;
#endif
        helper(0, &info, &ctx);
        _exit(2);
    }
    if (pid < 0) {
        std::cerr << "fork failed (illegal syscall)" << std::endl;
        return 7;
    }
    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        std::cerr << "waitpid failed (illegal syscall)" << std::endl;
        return 8;
    }
    if (WIFEXITED(status)) {
        if (WEXITSTATUS(status) != 1) {
            std::cerr << "illegal syscall exit code=" << WEXITSTATUS(status) << std::endl;
            return 9;
        }
    } else if (WIFSIGNALED(status)) {
        return 0;
    } else {
        std::cerr << "illegal syscall status=" << status << std::endl;
        return 9;
    }
    return 0;
}

static int run_seccomp_action_case(uint32_t action) {
    pid_t pid = fork();
    if (pid == 0) {
        if (install_helper()) {
            _exit(3);
        }
        scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_ALLOW);
        if (!ctx) _exit(4);
        if (seccomp_rule_add(ctx, action, SCMP_SYS(getpid), 0) != 0) _exit(5);
        if (seccomp_load(ctx) != 0) _exit(6);
        volatile int x = getpid();
        (void)x;
        _exit(0);
    }
    if (pid < 0) {
        std::cerr << "fork failed (seccomp action)" << std::endl;
        return 10;
    }
    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        std::cerr << "waitpid failed (seccomp action)" << std::endl;
        return 11;
    }
    if (action == SCMP_ACT_TRAP) {
        if (WIFEXITED(status) && WEXITSTATUS(status) == 1) {
            return 0;
        }
        if (WIFSIGNALED(status)) {
            return 1;
        }
        return 1;
    } else if (action == SCMP_ACT_KILL) {
        if (WIFSIGNALED(status)) {
            return 0;
        }
        return 1;
    }
    return 0;
}

static bool attempt_blocked_syscall() {
#if defined(SYS_socket)
    syscall(SYS_socket, AF_INET, SOCK_STREAM, 0);
    return true;
#elif defined(SYS_ptrace)
    syscall(SYS_ptrace, 0, 0, 0, 0);
    return true;
#elif defined(SYS_clone)
    syscall(SYS_clone, 0, 0, 0, 0, 0);
    return true;
#else
    return false;
#endif
}

static int run_blocked_syscall_case(const int* syscalls, bool restrict_execve, const char* name) {
    pid_t pid = fork();
    if (pid == 0) {
        char *args[] = {const_cast<char *>("prog"), nullptr};
        build_seccomp_filter(syscalls, restrict_execve, restrict_execve ? args : nullptr);
        if (!attempt_blocked_syscall()) {
            _exit(0);
        }
        _exit(2);
    }
    if (pid < 0) {
        std::cerr << "fork failed (" << name << " blocked syscall)" << std::endl;
        return 13;
    }
    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        std::cerr << "waitpid failed (" << name << " blocked syscall)" << std::endl;
        return 14;
    }
    if (WIFEXITED(status)) {
        int code = WEXITSTATUS(status);
        if (code == 1 || code == 0) {
            return 0;
        }
        std::cerr << name << " blocked syscall exit code=" << code << std::endl;
        return 15;
    }
    if (WIFSIGNALED(status)) {
        std::cerr << name << " blocked syscall signaled=" << WTERMSIG(status) << std::endl;
        return 16;
    }
    std::cerr << name << " blocked syscall status=" << status << std::endl;
    return 17;
}

int main() {
    auto root = make_temp_dir();
    int rc = run_basic_end_to_end(root);
    if (rc == 0) {
        rc = run_fixture_end_to_end(root);
    }
    if (rc == 0) {
        rc = run_illegal_syscall_case();
    }
    if (rc == 0) {
        int kill_rc = run_seccomp_action_case(SCMP_ACT_KILL);
        int trap_rc = run_seccomp_action_case(SCMP_ACT_TRAP);
        if (kill_rc != 0 || trap_rc != 0) {
            rc = 12;
        }
    }
    if (rc == 0) {
        int c11_rc = run_blocked_syscall_case(LANG_CV, true, "c11");
        int py3_rc = 0;
#ifdef LANG_PY3V
        py3_rc = run_blocked_syscall_case(LANG_PY3V, false, "python3");
#endif
        if (c11_rc != 0 || py3_rc != 0) {
            rc = 13;
        }
    }
    std::error_code ec;
    std::filesystem::remove_all(root, ec);
    return rc;
}
