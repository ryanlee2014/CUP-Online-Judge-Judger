/* eslint-disable no-console */
/**
 * Offline replay: re-judge historical submissions with judge v2 and compare with the verdicts stored in the
 * database. READ-ONLY: never writes the database and never touches data/<pid>/ac.
 *
 * usage: node dist/replay.js --db /home/CUP-Online-Judge-Express/config.json --out replay.jsonl
 *          [--sql "<WHERE clause>"] [--limit 500] [--concurrency 16] [--ids 1,2,3]
 */
import fs from "fs";
import { loadConfig, loadLanguages } from "./config";
import { GoJudgeClient } from "./gojudge";
import { Judge } from "./judge";
import { SubmissionInfo } from "./types";
// eslint-disable-next-line @typescript-eslint/no-var-requires
const mysql = require("mysql2/promise");

function arg(name: string, def?: string) {
	const i = process.argv.indexOf(`--${name}`);
	return i > 0 ? process.argv[i + 1] : def;
}

async function main() {
	const dbCfg = JSON.parse(fs.readFileSync(arg("db", "/home/CUP-Online-Judge-Express/config.json") as string, "utf8")).mysql;
	const pool = mysql.createPool({ ...dbCfg, connectionLimit: 4 });
	const limit = parseInt(arg("limit", "200") as string, 10);
	const ids = arg("ids");
	const where = ids ? `s.solution_id in (${ids.split(",").map(x => parseInt(x, 10)).join(",")})` : (arg("sql") || "1=1");
	const [rows] = await pool.query(
		`SELECT s.solution_id, s.problem_id, s.user_id, s.language, s.result, s.time, s.memory, s.pass_point,
		        p.time_limit, p.memory_limit, p.spj, c.source
		   FROM solution s JOIN problem p ON p.problem_id = s.problem_id JOIN source_code c ON c.solution_id = s.solution_id
		  WHERE s.problem_id > 0 AND s.result >= 4 AND s.result <= 11 AND ${where}
		  ORDER BY s.solution_id DESC LIMIT ?`, [limit]);
	await pool.end();
	console.error(`replaying ${rows.length} submissions`);

	const cfg = loadConfig();
	cfg.simEnable = false;
	const judge = new Judge(cfg, loadLanguages(cfg.languagesFile), new GoJudgeClient(cfg.goJudgeUrl));
	const out = fs.createWriteStream(arg("out", "replay.jsonl") as string);
	const conc = parseInt(arg("concurrency", "16") as string, 10);
	let next = 0, done = 0, same = 0;
	const t0 = Date.now();
	async function worker() {
		while (next < rows.length) {
			const r = rows[next++];
			const info: SubmissionInfo = {
				solution_id: r.solution_id, source: r.source, language: r.language, user_id: r.user_id,
				problem_id: r.problem_id, spj: r.spj, time_limit: Number(r.time_limit), memory_limit: Number(r.memory_limit)
			};
			const started = Date.now();
			const v2 = await judge.judge(info, () => undefined, { readOnly: true, noSim: true });
			const rec = {
				id: r.solution_id, pid: r.problem_id, lang: r.language, spj: r.spj,
				old: { state: r.result, time: r.time, memory: r.memory, pass: r.pass_point },
				v2: { state: v2.state, time: v2.time, memory: v2.memory, pass: v2.pass_point, total: v2.total_point,
					info: (v2.compile_info || v2.runtime_info || "").slice(0, 400) },
				same: r.result === v2.state, ms: Date.now() - started
			};
			if (rec.same) same++;
			out.write(JSON.stringify(rec) + "\n");
			done++;
			if (done % 50 === 0) console.error(`${done}/${rows.length} same=${same} ${Math.round((Date.now() - t0) / 1000)}s`);
		}
	}
	await Promise.all(Array.from({ length: conc }, worker));
	out.end();
	console.error(`done ${done} same=${same} (${(100 * same / Math.max(done, 1)).toFixed(1)}%) in ${Math.round((Date.now() - t0) / 1000)}s`);
}

main().catch(e => {
	console.error(e);
	process.exit(1);
});
