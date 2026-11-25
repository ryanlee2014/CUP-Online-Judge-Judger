/* eslint-disable no-console */
import { loadConfig, loadLanguages } from "./config";
import { GoJudgeClient } from "./gojudge";
import { Judge } from "./judge";
import { JudgeServer } from "./server";

async function main() {
	const cfg = loadConfig();
	const gj = new GoJudgeClient(cfg.goJudgeUrl);
	const v = await gj.version();
	console.log(`go-judge ${v.buildVersion} at ${cfg.goJudgeUrl}; languages from ${cfg.languagesFile}`);
	const judge = new Judge(cfg, loadLanguages(cfg.languagesFile), gj);
	new JudgeServer(cfg, judge).start();
}

main().catch(e => {
	console.error(e);
	process.exit(1);
});
