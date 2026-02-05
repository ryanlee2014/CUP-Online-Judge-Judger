#include "test_common.h"

#include <cstring>
#include <filesystem>
#include <iostream>

int main() {
    int failed = 0;
    auto base_cwd = std::filesystem::current_path();
    const char *filter = std::getenv("TEST_FILTER");
    for (const auto &t : tests()) {
        if (filter && !std::strstr(t.name, filter)) {
            continue;
        }
        std::filesystem::current_path(base_cwd);
        reset_globals_for_test();
        reset_globals_for_test();
        std::cout << "[RUN] " << t.name << "\n";
        try {
            t.func();
            std::cout << "[PASS] " << t.name << "\n";
        } catch (const TestFailure &e) {
            ++failed;
            std::cout << "[FAIL] " << t.name << ": " << e.msg << "\n";
        } catch (const test_hooks::ExitException &e) {
            ++failed;
            std::cout << "[FAIL] " << t.name << ": unexpected exit " << e.code << "\n";
        } catch (const std::exception &e) {
            ++failed;
            std::cout << "[FAIL] " << t.name << ": " << e.what() << "\n";
        }
    }
    if (failed) {
        std::cout << failed << " test(s) failed\n";
        return 1;
    }
    return 0;
}

