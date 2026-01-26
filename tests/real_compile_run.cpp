#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <cstdlib>

#include "../external/compare/CompareImpl.h"
#include "../header/static_var.h"
#include "../library/judge_lib.h"

int compare_zoj(const char *file1, const char *file2, int DEBUG, int full_diff);

static void write_file(const std::filesystem::path &path, const std::string &data) {
    auto parent = path.parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent);
    }
    std::ofstream out(path, std::ios::binary);
    out << data;
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

int main() {
    auto root = make_temp_dir();
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(root);

    write_file("Main.c",
               "#include <stdio.h>\n"
               "int main(){int a,b; if(scanf(\"%d%d\",&a,&b)!=2) return 1; printf(\"%d\\n\",a+b); return 0;}\n");
    if (run_command("cc -std=c11 -O2 -o main Main.c") != 0) {
        std::filesystem::current_path(old_cwd);
        return 1;
    }

    write_file("data.in", "1 2\n");
    write_file("data.out", "3\n");
    if (run_command("./main < data.in > user.out") != 0) {
        std::filesystem::current_path(old_cwd);
        return 2;
    }

    int res = compare_zoj("data.out", "user.out", 0, 0);
    std::filesystem::current_path(old_cwd);
    std::error_code ec;
    std::filesystem::remove_all(root, ec);
    return res == ACCEPT ? 0 : 3;
}
