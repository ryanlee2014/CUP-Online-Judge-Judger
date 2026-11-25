import fs from "fs";
import path from "path";
import { LanguageDef, WorkerConfig } from "./types";

const DEFAULTS: WorkerConfig = {
	port: 5111,
	host: "0.0.0.0",
	goJudgeUrl: "http://127.0.0.1:5050",
	ojHome: "/home/judge",
	judgerName: "RYZEN",
	concurrency: 24,
	languagesFile: "languages.json",
	compareBin: "cupoj-compare",
	tmpDir: "/tmp/cupoj-judge-v2",
	timeBonus: 2,
	memoryBonus: 128,
	fullDiff: true,
	simEnable: true,
	simScript: "/usr/bin/sim.sh",
	maxSubmissionSeconds: 900,
	runtimeInfoMax: 60000
};

export function loadConfig(file = process.env.JUDGE_V2_CONFIG || path.join(process.cwd(), "config.json")): WorkerConfig {
	const user = fs.existsSync(file) ? JSON.parse(fs.readFileSync(file, "utf8")) : {};
	const cfg: WorkerConfig = { ...DEFAULTS, ...user };
	const base = path.dirname(file);
	cfg.languagesFile = path.resolve(base, cfg.languagesFile);
	cfg.compareBin = path.resolve(base, cfg.compareBin);
	fs.mkdirSync(cfg.tmpDir, { recursive: true });
	return cfg;
}

export function loadLanguages(file: string): Record<string, LanguageDef> {
	const raw = JSON.parse(fs.readFileSync(file, "utf8"));
	delete raw._comment;
	return raw;
}
