#include "test_common.h"

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
