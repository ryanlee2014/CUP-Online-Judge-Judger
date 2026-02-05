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

namespace {
struct ParallelBudgetDecision {
    int workers = 1;
    int env_workers = -1;
    int memory_cap = -1;
    size_t total_files = 0;
};

ParallelBudgetDecision decide_parallel_budget(const ParallelRunOptions &opts) {
    ParallelBudgetDecision decision;
    decision.workers = max(int(thread::hardware_concurrency()), 1);
    if (!opts.in_file_list) {
        return decision;
    }
    decision.total_files = opts.in_file_list->size();
    if (decision.total_files == 0) {
        decision.workers = 1;
        return decision;
    }
    decision.env_workers = read_env_int("JUDGE_PARALLEL_WORKERS");
    if (decision.env_workers > 0) {
        decision.workers = decision.env_workers;
    }
    if (opts.memory_limit > 0) {
        // Conservative cap: assume each worker may need at least 64MB.
        decision.memory_cap = max(opts.memory_limit / 64, 1);
        decision.workers = min(decision.workers, decision.memory_cap);
    }
    decision.workers = max(decision.workers, 1);
    if (static_cast<size_t>(decision.workers) > decision.total_files) {
        decision.workers = static_cast<int>(decision.total_files);
    }
    return decision;
}

void log_parallel_budget_decision(const ParallelRunOptions &opts, const ParallelBudgetDecision &decision,
                                  size_t chunk_size, int env_chunk) {
    if (!opts.debug_enabled || !opts.env || opts.env->oj_home.empty()) {
        return;
    }
    write_log(opts.env->oj_home.c_str(),
              "parallel budget workers=%d total=%zu env_workers=%d memory_cap=%d chunk=%zu env_chunk=%d",
              decision.workers, decision.total_files, decision.env_workers, decision.memory_cap, chunk_size, env_chunk);
}
}  // namespace

int compute_parallel_budget(const ParallelRunOptions &opts) {
    return decide_parallel_budget(opts).workers;
}

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

JudgeResult runJudgeTask(const RunTaskOptions &opts) {
    int call_counter_local[call_array_size], PEflg;
    char infile[BUFFER_SIZE], outfile[BUFFER_SIZE], userfile[BUFFER_SIZE];
    int topmemory = 0;
    prepare_run_files_with_id(opts.language, opts.runner_id, *opts.infile_pair, opts.problem_id, opts.work_dir,
                              opts.case_index, call_counter_local, infile, outfile, userfile, opts.syscall_template,
                              opts.record_syscall);
    double usedtime = opts.used_time;
    double timeLimit = opts.time_limit;
    int memoryLimit = opts.memory_limit;
    int language = opts.language;
    auto pid = spawn_child([&]() {
        run_solution_parallel(language, opts.work_dir, timeLimit, usedtime, memoryLimit, opts.case_index, *opts.config,
                              opts.language_factory);
    });
    if (pid == CHILD_PROCESS) {
        return {};
    } else {
        int ACflg = opts.ACflg;
        return finish_run_with_id(pid, ACflg, opts.special_judge, opts.solution_id, opts.language, topmemory,
                                  opts.memory_limit, usedtime, opts.time_limit, opts.problem_id, PEflg, opts.work_dir,
                                  opts.case_index, call_counter_local, infile, outfile, userfile, opts.usercode,
                                  *opts.global_work_dir, *opts.config, *opts.env, opts.record_syscall,
                                  opts.debug_enabled, opts.language_factory, opts.compare_factory);
    }
}

JudgeResult runJudgeTask(int runner_id, int language, char *work_dir, const pair<string, int> &infilePair, int ACflg,
                         int SPECIAL_JUDGE, int solution_id, double timeLimit, double usedtime, int memoryLimit,
                         int problemId, char *usercode, int num_of_test, string &global_work_dir,
                         const JudgeConfigSnapshot &config, const JudgeEnv &env,
                         bool record_syscall, bool debug_enabled, const int *syscall_template,
                         const LanguageFactory &language_factory, const CompareFactory &compare_factory) {
    RunTaskOptions opts;
    opts.runner_id = runner_id;
    opts.language = language;
    opts.work_dir = work_dir;
    opts.infile_pair = &infilePair;
    opts.ACflg = ACflg;
    opts.special_judge = SPECIAL_JUDGE;
    opts.solution_id = solution_id;
    opts.time_limit = timeLimit;
    opts.used_time = usedtime;
    opts.memory_limit = memoryLimit;
    opts.problem_id = problemId;
    opts.usercode = usercode;
    opts.case_index = num_of_test;
    opts.global_work_dir = &global_work_dir;
    opts.config = &config;
    opts.env = &env;
    opts.record_syscall = record_syscall;
    opts.debug_enabled = debug_enabled;
    opts.syscall_template = syscall_template;
    opts.language_factory = language_factory;
    opts.compare_factory = compare_factory;
    return runJudgeTask(opts);
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

JudgeSeriesResult runParallelJudge(const ParallelRunOptions &opts) {
    struct ChunkResult {
        vector<JudgeResult> results;
    };
    ParallelBudgetDecision budget = decide_parallel_budget(opts);
    int workers = budget.workers;
    auto &inFileList = *opts.in_file_list;
    size_t total = inFileList.size();
    if (total == 0) {
        return {3, 0, 0, 0, 0};
    }
    JudgeSeriesResult finalResult = {3, 0, 0, 0, 0};
#ifdef UNIT_TEST
    for (size_t i = 0; i < total; ++i) {
        JudgeResult r{};
        r.ACflg = *opts.ac_flag;
        r.usedTime = 0;
        r.topMemory = 0;
        r.num = static_cast<int>(i);
        update_series_result(finalResult, r);
    }
    return finalResult;
#endif
    ThreadPool pool(workers);
    vector<future<ChunkResult>> result;
    size_t chunk_size = (total + workers - 1) / workers;
    int env_chunk = read_env_int("JUDGE_PARALLEL_CHUNK");
    if (env_chunk > 0) {
        chunk_size = static_cast<size_t>(env_chunk);
    }
    chunk_size = max<size_t>(1, min(chunk_size, total));
    log_parallel_budget_decision(opts, budget, chunk_size, env_chunk);
    const int base_ac = *opts.ac_flag;
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
                JudgeResult r = runJudgeTask(opts.runner_id, opts.submission->getLanguage(), opts.work_dir, infilePair,
                                             local_ac, opts.special_judge, opts.submission->getSolutionId(),
                                             opts.submission->getTimeLimit(), usedtime_local,
                                             opts.submission->getMemoryLimit(), opts.submission->getProblemId(),
                                             opts.usercode, int(i), *opts.global_work_dir, *opts.config, *opts.env,
                                             opts.record_syscall, opts.debug_enabled,
                                             opts.syscall_template, opts.language_factory, opts.compare_factory);
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
                                   const int *syscall_template, const LanguageFactory &language_factory,
                                   const CompareFactory &compare_factory) {
    ParallelRunOptions opts;
    opts.runner_id = runner_id;
    opts.language = language;
    opts.work_dir = work_dir;
    opts.usercode = usercode;
    opts.time_limit = timeLimit;
    opts.used_time = usedtime;
    opts.memory_limit = memoryLimit;
    opts.in_file_list = &inFileList;
    opts.ac_flag = &ACflg;
    opts.special_judge = SPECIAL_JUDGE;
    opts.global_work_dir = &global_work_dir;
    opts.submission = &submissionInfo;
    opts.config = &config;
    opts.env = &env;
    opts.record_syscall = record_syscall;
    opts.debug_enabled = debug_enabled;
    opts.syscall_template = syscall_template;
    opts.language_factory = language_factory;
    opts.compare_factory = compare_factory;
    return runParallelJudge(opts);
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
