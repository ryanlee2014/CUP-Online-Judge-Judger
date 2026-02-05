#include "test_common.h"

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
