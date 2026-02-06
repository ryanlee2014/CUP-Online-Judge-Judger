#include "judge_client_util.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "header/static_var.h"
#ifdef UNIT_TEST
#include "tests/test_hooks.h"
#endif
#include "judge_client_runtime_bridge.h"
#include "judge_client_util_helpers.h"
#include "library/judge_lib.h"

using namespace std;

extern int call_counter[call_array_size];

void init_parameters(int argc, char **argv, int &solution_id,
                     int &runner_id, string &judgerId) {
    if (argc < 3) {
        bool silent = false;
#ifdef UNIT_TEST
        silent = test_hooks::state().exit_throws;
#endif
        if (!silent) {
            fprintf(stderr, "Usage:%s solution_id runner_id.\n", argv[0]);
            fprintf(stderr, "Multi:%s solution_id runner_id judge_base_path.\n",
                    argv[0]);
            fprintf(stderr,
                    "Debug:%s solution_id runner_id judge_base_path debug.\n",
                    argv[0]);
        }
#ifdef UNIT_TEST
        test_exit(1);
        return;
#else
        exit(1);
#endif
    }
    judge_util_helpers::ParsedArgs parsed;
    if (judge_util_helpers::parse_new_args(argc, argv, parsed)) {
        judge_util_helpers::InitRuntimeConfig runtime = judge_util_helpers::build_runtime_from_parsed(parsed);
        judge_util_helpers::apply_runtime_to_outputs(runtime, solution_id, runner_id, judgerId);
        apply_bootstrap_globals_from_runtime(runtime);
        return;
    }
    judge_util_helpers::InitRuntimeConfig runtime = judge_util_helpers::build_runtime_from_legacy(argc, argv);
    judge_util_helpers::apply_runtime_to_outputs(runtime, solution_id, runner_id, judgerId);
    apply_bootstrap_globals_from_runtime(runtime);
}

void print_call_array() {
    printf("int LANG_%sV[256]={", LANG_NAME);
    int i = 0;
    for (i = 0; i < call_array_size; i++) {
        if (call_counter[i]) {
            printf("%d,", i);
        }
    }
    printf("0};\n");

    printf("int LANG_%sC[256]={", LANG_NAME);
    for (i = 0; i < call_array_size; i++) {
        if (call_counter[i]) {
            printf("HOJ_MAX_LIMIT,");
        }
    }
    printf("0};\n");
}
