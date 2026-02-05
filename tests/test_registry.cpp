#include "test_registry.h"

std::vector<TestCase> &tests() {
    static std::vector<TestCase> t;
    return t;
}

bool register_test(const char *name, TestFunc func) {
    tests().push_back({name, func});
    return true;
}
