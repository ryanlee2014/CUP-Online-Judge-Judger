#include "test_common.h"

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
