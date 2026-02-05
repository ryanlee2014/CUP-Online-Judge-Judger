#include "test_common.h"

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
