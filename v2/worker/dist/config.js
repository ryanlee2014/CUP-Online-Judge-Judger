"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.loadConfig = loadConfig;
exports.loadLanguages = loadLanguages;
const fs_1 = __importDefault(require("fs"));
const path_1 = __importDefault(require("path"));
const DEFAULTS = {
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
function loadConfig(file = process.env.JUDGE_V2_CONFIG || path_1.default.join(process.cwd(), "config.json")) {
    const user = fs_1.default.existsSync(file) ? JSON.parse(fs_1.default.readFileSync(file, "utf8")) : {};
    const cfg = { ...DEFAULTS, ...user };
    const base = path_1.default.dirname(file);
    cfg.languagesFile = path_1.default.resolve(base, cfg.languagesFile);
    cfg.compareBin = path_1.default.resolve(base, cfg.compareBin);
    fs_1.default.mkdirSync(cfg.tmpDir, { recursive: true });
    return cfg;
}
function loadLanguages(file) {
    const raw = JSON.parse(fs_1.default.readFileSync(file, "utf8"));
    delete raw._comment;
    return raw;
}
//# sourceMappingURL=config.js.map