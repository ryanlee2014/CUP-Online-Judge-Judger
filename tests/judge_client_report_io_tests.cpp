#include "test_common.h"

#include "../judge_client_report_finalize_helpers.h"
#include "../judge_client_report_test_run_helpers.h"
#include "../judge_client_run_io_helpers.h"

namespace {
struct StdFdGuard {
    int stdin_fd = -1;
    int stdout_fd = -1;
    int stderr_fd = -1;

    StdFdGuard() {
        stdin_fd = dup(fileno(stdin));
        stdout_fd = dup(fileno(stdout));
        stderr_fd = dup(fileno(stderr));
    }

    ~StdFdGuard() {
        if (stdin_fd >= 0) {
            fflush(stdin);
            dup2(stdin_fd, fileno(stdin));
            close(stdin_fd);
        }
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
};
}  // namespace

TEST(ReportFinalizeResolveFinalResult) {
    JudgeConfigSnapshot config;
    config.all_test_mode = 0;
    EXPECT_EQ(resolve_final_result(ACCEPT, WRONG_ANSWER, 0, config), ACCEPT);

    config.all_test_mode = 1;
    EXPECT_EQ(resolve_final_result(ACCEPT, WRONG_ANSWER, 0, config), WRONG_ANSWER);
    EXPECT_EQ(resolve_final_result(ACCEPT, ACCEPT, PRESENTATION_ERROR, config), PRESENTATION_ERROR);
}

TEST(ReportFinalizeApplyRuntimeInfoAndTime) {
    TempDir tmp;
    write_file(tmp.path / "error.out", "runtime");
    write_file(tmp.path / "diff.out", "diff");

    JudgeConfigSnapshot config;
    double usedtime = 123.0;
    std::string runtime_info;

    apply_runtime_info_and_time(RUNTIME_ERROR, 1, config, tmp.path.string().c_str(), false, 1.0, 456.0,
                                usedtime, runtime_info);
    EXPECT_EQ(runtime_info, "runtime");
    EXPECT_EQ(usedtime, 123.0);

    apply_runtime_info_and_time(WRONG_ANSWER, 1, config, tmp.path.string().c_str(), false, 1.0, 456.0,
                                usedtime, runtime_info);
    EXPECT_EQ(runtime_info, "diff");

    config.use_max_time = 1;
    usedtime = 123.0;
    apply_runtime_info_and_time(ACCEPT, 1, config, tmp.path.string().c_str(), false, 1.0, 456.0,
                                usedtime, runtime_info);
    EXPECT_EQ(usedtime, 456.0);

    usedtime = 123.0;
    apply_runtime_info_and_time(TIME_LIMIT_EXCEEDED, 1, config, tmp.path.string().c_str(), false, 1.0, 456.0,
                                usedtime, runtime_info);
    EXPECT_EQ(usedtime, 1000.0);
}

TEST(ReportTestRunOutputBranches) {
    TempDir tmp;
    write_file(tmp.path / "user.out", "ok");

    double usedtime = 0;
    std::string out = build_test_run_output(ACCEPT, usedtime, 1.0, 1, tmp.path.string().c_str(), false);
    EXPECT_EQ(out, "ok");

    usedtime = 0;
    out = build_test_run_output(TIME_LIMIT_EXCEEDED, usedtime, 1.0, 1, tmp.path.string().c_str(), false);
    EXPECT_TRUE(out.find("Time Limit Exceeded") != std::string::npos);
    EXPECT_TRUE(out.size() > std::string("Time Limit Exceeded.Kill Process.\n").size());
    EXPECT_EQ(usedtime, 1000.0);

    usedtime = 0;
    out = build_test_run_output(RUNTIME_ERROR, usedtime, 1.0, 1, tmp.path.string().c_str(), true);
    EXPECT_TRUE(out.find("Runtime Error") != std::string::npos);

    usedtime = 0;
    out = build_test_run_output(MEMORY_LIMIT_EXCEEDED, usedtime, 1.0, 1, tmp.path.string().c_str(), false);
    EXPECT_TRUE(out.find("Memory Limit Exceeded") != std::string::npos);
}

TEST(ReportTestRunOutputOmit) {
    TempDir tmp;
    std::string big(5000, 'a');
    write_file(tmp.path / "user.out", big);
    double usedtime = 0;
    std::string out = build_test_run_output(ACCEPT, usedtime, 1.0, 1, tmp.path.string().c_str(), false);
    EXPECT_TRUE(out.find("Omit") != std::string::npos);
    EXPECT_TRUE(out.size() > 4096);
}

TEST(RunIoPreparePaths) {
    std::string in;
    std::string out;
    std::string err;
    judge_run_helpers::prepare_io_paths("/tmp/work", "data.in", "user.out", "error.out", in, out, err);
    EXPECT_EQ(in, "/tmp/work/data.in");
    EXPECT_EQ(out, "/tmp/work/user.out");
    EXPECT_EQ(err, "/tmp/work/error.out");
}

TEST(RunIoRedirectStdioSuccess) {
    StdFdGuard guard;
    TempDir tmp;
    auto in = (tmp.path / "data.in").string();
    auto out = (tmp.path / "user.out").string();
    auto err = (tmp.path / "error.out").string();
    write_file(in, "abc\n");

    judge_run_helpers::redirect_stdio(in, out, err);
    char buf[16] = {};
    fgets(buf, sizeof(buf), stdin);
    EXPECT_EQ(std::string(buf), "abc\n");
    std::fputs("hello", stdout);
    std::fputs("oops", stderr);
    std::fflush(stdout);
    std::fflush(stderr);

    EXPECT_EQ(read_file(out), "hello");
    EXPECT_EQ(read_file(err), "oops");
}

TEST(RunIoRedirectStdioFailureBranches) {
    TempDir tmp;
    auto in = (tmp.path / "data.in").string();
    auto out = (tmp.path / "user.out").string();
    auto err = (tmp.path / "error.out").string();
    write_file(in, "abc");

    {
        StdFdGuard guard;
        expect_exit([&]() {
            judge_run_helpers::redirect_stdio((tmp.path / "missing.in").string(), out, err);
        }, 1);
    }
    {
        StdFdGuard guard;
        expect_exit([&]() {
            judge_run_helpers::redirect_stdio(in, (tmp.path / "no_dir" / "user.out").string(), err);
        }, 1);
    }
    {
        StdFdGuard guard;
        expect_exit([&]() {
            judge_run_helpers::redirect_stdio(in, out, (tmp.path / "no_dir" / "error.out").string());
        }, 1);
    }
}
