"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
/* eslint-disable no-console */
const config_1 = require("./config");
const gojudge_1 = require("./gojudge");
const judge_1 = require("./judge");
const server_1 = require("./server");
async function main() {
    const cfg = (0, config_1.loadConfig)();
    const gj = new gojudge_1.GoJudgeClient(cfg.goJudgeUrl);
    const v = await gj.version();
    console.log(`go-judge ${v.buildVersion} at ${cfg.goJudgeUrl}; languages from ${cfg.languagesFile}`);
    const judge = new judge_1.Judge(cfg, (0, config_1.loadLanguages)(cfg.languagesFile), gj);
    new server_1.JudgeServer(cfg, judge).start();
}
main().catch(e => {
    console.error(e);
    process.exit(1);
});
//# sourceMappingURL=main.js.map