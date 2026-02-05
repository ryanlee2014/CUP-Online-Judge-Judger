#pragma once

#include <string>
#include <vector>

struct TestFailure {
    std::string msg;
};

using TestFunc = void (*)();

struct TestCase {
    const char *name;
    TestFunc func;
};

std::vector<TestCase> &tests();
bool register_test(const char *name, TestFunc func);
