#include "judge_client_run.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <sys/types.h>

#include "header/static_var.h"
#include "judge_client_context.h"
#include "judge_client_process_utils.h"
#include "judge_client_run_helpers.h"
#include "judge_client_watch.h"
#include "library/judge_lib.h"
#include "model/base/ThreadPool.h"
#include "model/judge/language/Language.h"

using namespace std;
using namespace judge_run_helpers;

void run_solution_parallel(int &lang, char *work_dir, double &time_lmt, double &usedtime,
                           int &mem_lmt, int fileId, const JudgeConfigSnapshot &config) {
    char input[BUFFER_SIZE], userOutput[BUFFER_SIZE], errorOutput[BUFFER_SIZE];
    build_parallel_io_names(fileId, input, userOutput, errorOutput);
    run_solution_common(lang, work_dir, time_lmt, usedtime, mem_lmt, input, userOutput, errorOutput, 1.0, 1.0,
                        config);
}

void run_solution_parallel(int &lang, char *work_dir, double &time_lmt, double &usedtime,
                           int &mem_lmt, int fileId, const JudgeConfigSnapshot &config,
                           const LanguageFactory &language_factory) {
    char input[BUFFER_SIZE], userOutput[BUFFER_SIZE], errorOutput[BUFFER_SIZE];
    build_parallel_io_names(fileId, input, userOutput, errorOutput);
    run_solution_common(lang, work_dir, time_lmt, usedtime, mem_lmt, input, userOutput, errorOutput, 1.0, 1.0,
                        config, language_factory);
}

void run_solution(int &lang, char *work_dir, double &time_lmt, double &usedtime,
                  int &mem_lmt, const JudgeConfigSnapshot &config) {
    run_solution_common(lang, work_dir, time_lmt, usedtime, mem_lmt, "data.in", "user.out", "error.out", 1.5, 2.0,
                        config);
}

void run_solution(int &lang, char *work_dir, const double &time_lmt, const double &usedtime,
                  const int &mem_lmt, const JudgeConfigSnapshot &config) {
    run_solution_common(lang, work_dir, time_lmt, usedtime, mem_lmt, "data.in", "user.out", "error.out", 1.5, 2.0,
                        config);
}

void run_solution(int &lang, char *work_dir, double &time_lmt, double &usedtime,
                  int &mem_lmt, const JudgeConfigSnapshot &config, const LanguageFactory &language_factory) {
    run_solution_common(lang, work_dir, time_lmt, usedtime, mem_lmt, "data.in", "user.out", "error.out", 1.5, 2.0,
                        config, language_factory);
}

void run_solution(int &lang, char *work_dir, const double &time_lmt, const double &usedtime,
                  const int &mem_lmt, const JudgeConfigSnapshot &config, const LanguageFactory &language_factory) {
    run_solution_common(lang, work_dir, time_lmt, usedtime, mem_lmt, "data.in", "user.out", "error.out", 1.5, 2.0,
                        config, language_factory);
}

JudgeResult runJudgeTask(int runner_id, int language, char *work_dir, const pair<string, int> &infilePair, int ACflg,
                         int SPECIAL_JUDGE, int solution_id, double timeLimit, double usedtime, int memoryLimit,
                         int problemId, char *usercode, int num_of_test, string &global_work_dir,
                         const JudgeConfigSnapshot &config, const JudgeEnv &env,
                         bool record_syscall, bool debug_enabled, const int *syscall_template,
                         const LanguageFactory &language_factory, const CompareFactory &compare_factory) {
    int call_counter_local[call_array_size], PEflg;
    char infile[BUFFER_SIZE], outfile[BUFFER_SIZE], userfile[BUFFER_SIZE];
    int topmemory = 0;
    prepare_run_files_with_id(language, runner_id, infilePair, problemId, work_dir, num_of_test, call_counter_local,
                              infile, outfile, userfile, syscall_template, record_syscall);
    auto pid = spawn_child([&]() {
        run_solution_parallel(language, work_dir, timeLimit, usedtime, memoryLimit, num_of_test, config,
                              language_factory);
    });
    if (pid == CHILD_PROCESS) {
        return {};
    } else {
        return finish_run_with_id(pid, ACflg, SPECIAL_JUDGE, solution_id, language, topmemory, memoryLimit, usedtime,
                                  timeLimit, problemId, PEflg, work_dir, num_of_test, call_counter_local, infile,
                                  outfile, userfile, usercode, global_work_dir, config, env,
                                  record_syscall, debug_enabled, language_factory, compare_factory);
    }
}

JudgeResult runJudgeTask(int runner_id, int language, char *work_dir, const pair<string, int> &infilePair, int ACflg,
                         int SPECIAL_JUDGE, int solution_id, double timeLimit, double usedtime, int memoryLimit,
                         int problemId, char *usercode, int num_of_test, string &global_work_dir,
                         const JudgeConfigSnapshot &config, const JudgeEnv &env,
                         bool record_syscall, bool debug_enabled, const int *syscall_template) {
    return runJudgeTask(runner_id, language, work_dir, infilePair, ACflg, SPECIAL_JUDGE,
                        solution_id, timeLimit, usedtime, memoryLimit,
                        problemId, usercode, num_of_test, global_work_dir, config, env,
                        record_syscall, debug_enabled, syscall_template, nullptr, nullptr);
}

JudgeResult runJudgeTask(int runner_id, int language, char *work_dir, const pair<string, int> &infilePair, int ACflg,
                         int SPECIAL_JUDGE, int solution_id, double timeLimit, double usedtime, int memoryLimit,
                         int problemId, char *usercode, int num_of_test, string &global_work_dir,
                         const JudgeConfigSnapshot &config, const JudgeEnv &env,
                         bool record_syscall, bool debug_enabled, const int *syscall_template,
                         const LanguageFactory &language_factory) {
    return runJudgeTask(runner_id, language, work_dir, infilePair, ACflg, SPECIAL_JUDGE,
                        solution_id, timeLimit, usedtime, memoryLimit,
                        problemId, usercode, num_of_test, global_work_dir, config, env,
                        record_syscall, debug_enabled, syscall_template, language_factory, nullptr);
}

JudgeResult runJudgeTask(int runner_id, int language, char *work_dir, const pair<string, int> &infilePair, int ACflg,
                         int SPECIAL_JUDGE, int solution_id, double timeLimit, double usedtime, int memoryLimit,
                         int problemId, char *usercode, int num_of_test, string &global_work_dir,
                         const JudgeConfigSnapshot &config, const JudgeEnv &env,
                         bool record_syscall, bool debug_enabled) {
    return runJudgeTask(runner_id, language, work_dir, infilePair, ACflg, SPECIAL_JUDGE,
                        solution_id, timeLimit, usedtime, memoryLimit,
                        problemId, usercode, num_of_test, global_work_dir, config, env,
                        record_syscall, debug_enabled, nullptr, nullptr, nullptr);
}

JudgeSeriesResult runParallelJudge(int runner_id, int language, char *work_dir, char *usercode, int timeLimit,
                                   int usedtime, int memoryLimit,
                                   vector<pair<string, int>> &inFileList,
                                   int &ACflg, int SPECIAL_JUDGE, string &global_work_dir,
                                   SubmissionInfo &submissionInfo, const JudgeConfigSnapshot &config,
                                   const JudgeEnv &env, bool record_syscall, bool debug_enabled,
                                   const int *syscall_template, const LanguageFactory &language_factory,
                                   const CompareFactory &compare_factory) {
    struct ChunkResult {
        vector<JudgeResult> results;
    };
    int workers = max(int(std::thread::hardware_concurrency()), 1);
    size_t total = inFileList.size();
    if (total == 0) {
        return {3, 0, 0, 0, 0};
    }
    JudgeSeriesResult finalResult = {3, 0, 0, 0, 0};
#ifdef UNIT_TEST
    for (size_t i = 0; i < total; ++i) {
        JudgeResult r{};
        r.ACflg = ACflg;
        r.usedTime = 0;
        r.topMemory = 0;
        r.num = static_cast<int>(i);
        update_series_result(finalResult, r);
    }
    return finalResult;
#endif
    int env_workers = read_env_int("JUDGE_PARALLEL_WORKERS");
    if (env_workers > 0) {
        workers = env_workers;
    }
    if (total > 0 && static_cast<size_t>(workers) > total) {
        workers = static_cast<int>(total);
    }
    ThreadPool pool(workers);
    vector<future<ChunkResult>> result;
    size_t chunk_size = (total + workers - 1) / workers;
    int env_chunk = read_env_int("JUDGE_PARALLEL_CHUNK");
    if (env_chunk > 0) {
        chunk_size = static_cast<size_t>(env_chunk);
    }
    chunk_size = max<size_t>(1, min(chunk_size, total));
    const int base_ac = ACflg;
    result.reserve((total + chunk_size - 1) / chunk_size);
    for (size_t start = 0; start < total; start += chunk_size) {
        size_t end = min(start + chunk_size, total);
        result.emplace_back(pool.enqueue([&, start, end]() {
            ChunkResult chunk;
            chunk.results.reserve(end - start);
            for (size_t i = start; i < end; ++i) {
                double usedtime_local = 0;
                const auto &infilePair = inFileList[i];
                int local_ac = base_ac;
                JudgeResult r = runJudgeTask(runner_id, submissionInfo.getLanguage(), work_dir, infilePair,
                                             local_ac, SPECIAL_JUDGE, submissionInfo.getSolutionId(),
                                             submissionInfo.getTimeLimit(), usedtime_local,
                                             submissionInfo.getMemoryLimit(), submissionInfo.getProblemId(),
                                             usercode, int(i), global_work_dir, config, env,
                                             record_syscall, debug_enabled,
                                             syscall_template, language_factory, compare_factory);
                chunk.results.push_back(r);
            }
            return chunk;
        }));
    }
    for (auto &res: result) {
        ChunkResult chunk = res.get();
        for (auto &r: chunk.results) {
            update_series_result(finalResult, r);
        }
    }
    return finalResult;
}

JudgeSeriesResult runParallelJudge(int runner_id, int language, char *work_dir, char *usercode, int timeLimit,
                                   int usedtime, int memoryLimit,
                                   vector<pair<string, int>> &inFileList,
                                   int &ACflg, int SPECIAL_JUDGE, string &global_work_dir,
                                   SubmissionInfo &submissionInfo, const JudgeConfigSnapshot &config,
                                   const JudgeEnv &env, bool record_syscall, bool debug_enabled,
                                   const int *syscall_template, const LanguageFactory &language_factory) {
    return runParallelJudge(runner_id, language, work_dir, usercode, timeLimit, usedtime, memoryLimit, inFileList,
                            ACflg, SPECIAL_JUDGE, global_work_dir, submissionInfo, config, env,
                            record_syscall, debug_enabled, syscall_template, language_factory, nullptr);
}

JudgeSeriesResult runParallelJudge(int runner_id, int language, char *work_dir, char *usercode, int timeLimit,
                                   int usedtime, int memoryLimit,
                                   vector<pair<string, int>> &inFileList,
                                   int &ACflg, int SPECIAL_JUDGE, string &global_work_dir,
                                   SubmissionInfo &submissionInfo, const JudgeConfigSnapshot &config,
                                   const JudgeEnv &env, bool record_syscall, bool debug_enabled) {
    return runParallelJudge(runner_id, language, work_dir, usercode, timeLimit, usedtime, memoryLimit, inFileList,
                            ACflg, SPECIAL_JUDGE, global_work_dir, submissionInfo, config, env,
                            record_syscall, debug_enabled, nullptr, nullptr, nullptr);
}

JudgeSeriesResult runParallelJudge(int runner_id, int language, char *work_dir, char *usercode, int timeLimit,
                                   int usedtime, int memoryLimit,
                                   vector<pair<string, int>> &inFileList,
                                   int &ACflg, int SPECIAL_JUDGE, string &global_work_dir,
                                   SubmissionInfo &submissionInfo, const JudgeConfigSnapshot &config,
                                   const JudgeEnv &env, bool record_syscall, bool debug_enabled,
                                   const int *syscall_template) {
    return runParallelJudge(runner_id, language, work_dir, usercode, timeLimit, usedtime, memoryLimit, inFileList,
                            ACflg, SPECIAL_JUDGE, global_work_dir, submissionInfo, config, env,
                            record_syscall, debug_enabled, syscall_template, nullptr, nullptr);
}
