#include "judge_client_run.h"

#include <iostream>
#include <memory>

#include "external/compare/Compare.h"
#include "judge_client_context.h"
#include "library/judge_lib.h"
#include "model/judge/language/Language.h"
#include "model/judge/policy/SpecialJudge.h"

using namespace std;

namespace {
shared_ptr<Language> resolve_language_model(JudgeContext &ctx, int lang) {
    if (ctx.language_model) {
        return ctx.language_model;
    }
    if (ctx.language_factory) {
        return ctx.language_factory(lang);
    }
    return shared_ptr<Language>(getLanguageModel(lang));
}

shared_ptr<Compare::Compare> resolve_compare_model(JudgeContext &ctx) {
    if (ctx.compare_factory) {
        return ctx.compare_factory();
    }
    return shared_ptr<Compare::Compare>(getCompareModel());
}
} // namespace

JudgeResult judge_solution(int &ACflg, double &usedtime, double time_lmt, int isspj,
                           int p_id, char *infile, char *outfile, char *userfile, char *usercode, int &PEflg,
                           int lang, char *work_dir, int &topmemory, int mem_lmt,
                           int solution_id, int num_of_test, string &global_work_dir,
                           const JudgeConfigSnapshot &config, const JudgeEnv &env, bool debug_enabled) {
    JudgeContext ctx;
    ctx.lang = lang;
    ctx.p_id = p_id;
    ctx.config = config;
    ctx.env = env;
    ctx.flags.debug = debug_enabled ? 1 : 0;
    return judge_solution(ctx, ACflg, usedtime, time_lmt, isspj, infile, outfile, userfile, usercode, PEflg,
                          work_dir, topmemory, mem_lmt, solution_id, num_of_test, global_work_dir);
}

JudgeResult judge_solution(JudgeContext &ctx, int &ACflg, double &usedtime, double time_lmt, int isspj,
                           char *infile, char *outfile, char *userfile, char *usercode, int &PEflg,
                           char *work_dir, int &topmemory, int mem_lmt,
                           int solution_id, int num_of_test, string &global_work_dir) {
    auto languageModel = resolve_language_model(ctx, ctx.lang);
    cout << "Used time" << endl;
    cout << usedtime << endl;
    cout << time_lmt * 1000 * (ctx.config.use_max_time ? 1 : num_of_test) << endl;
    cout << "judge solution: infile: " << infile << " outfile: " << outfile << " userfile: " << userfile << endl;
    int comp_res;
    if (!ctx.config.all_test_mode) {
        num_of_test = static_cast<int>(1.0);
    }
    if (ACflg == ACCEPT && usedtime > time_lmt * 1000 * (ctx.config.use_max_time ? 1 : num_of_test)) {
        cout << "Time Limit Exceeded" << endl;
        usedtime = time_lmt * 1000;
        ACflg = TIME_LIMIT_EXCEEDED;
    }
    if (topmemory > mem_lmt * STD_MB) {
        ACflg = MEMORY_LIMIT_EXCEEDED;
    }
    languageModel->fixACFlag(ACflg);
    if (ACflg == ACCEPT) {
        if (isspj) {
            std::string oj_home_path = ctx.env.oj_home;
            comp_res = SpecialJudge::newInstance().setDebug(ctx.flags.debug != 0).run(oj_home_path.data(), ctx.p_id,
                                                                                      infile, outfile, userfile,
                                                                                      usercode, global_work_dir);
        } else {
            auto compare = resolve_compare_model(ctx);
            compare->setDebug(ctx.flags.debug != 0);
            comp_res = compare->compare(outfile, userfile);
        }
        if (comp_res == WRONG_ANSWER) {
            ACflg = WRONG_ANSWER;
            if (ctx.flags.debug) {
                printf("fail test %s\n", infile);
            }
        } else if (comp_res == PRESENTATION_ERROR) {
            PEflg = PRESENTATION_ERROR;
        }
        ACflg = comp_res;
    }
    languageModel->fixFlagWithVMIssue(work_dir, ACflg, topmemory, mem_lmt);
    return {ACflg, usedtime, topmemory, 0};
}
