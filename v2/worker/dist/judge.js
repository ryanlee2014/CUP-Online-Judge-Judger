"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.Judge = void 0;
exports.listTestCases = listTestCases;
const fs_1 = __importDefault(require("fs"));
const path_1 = __importDefault(require("path"));
const child_process_1 = require("child_process");
const util_1 = require("util");
const gojudge_1 = require("./gojudge");
const execFileAsync = (0, util_1.promisify)(child_process_1.execFile);
const COMPILE_OUT_MAX = 64 * 1024;
const STDERR_MAX = 64 * 1024;
/** sort "test2.in" before "test10.in" */
function naturalCompare(a, b) {
    return a.localeCompare(b, "en", { numeric: true });
}
function listTestCases(dataDir) {
    const names = fs_1.default.readdirSync(dataDir).filter(f => f.length > 3 && f.endsWith(".in"));
    const cases = [];
    for (const f of names) {
        const stem = f.slice(0, -3);
        const ans = path_1.default.join(dataDir, stem + ".out");
        if (fs_1.default.existsSync(ans)) {
            cases.push({ name: stem, input: path_1.default.join(dataDir, f), answer: ans });
        }
    }
    cases.sort((x, y) => naturalCompare(x.name, y.name));
    return cases;
}
function truncate(s, max) {
    if (s.length <= max)
        return s;
    return s.slice(0, max) + `\n......Omit ${s.length - max} characters.`;
}
function signalName(code) {
    const names = { 6: "Aborted", 8: "Floating point exception", 9: "Killed", 11: "Segmentation fault", 7: "Bus error", 4: "Illegal instruction", 13: "Broken pipe", 31: "Bad system call" };
    return names[code] || `Signal ${code}`;
}
class Judge {
    cfg;
    languages;
    gj;
    constructor(cfg, languages, gj) {
        this.cfg = cfg;
        this.languages = languages;
        this.gj = gj;
    }
    language(id) {
        return this.languages[String(id)];
    }
    dataDir(problemId) {
        return path_1.default.join(this.cfg.ojHome, "data", String(Math.abs(problemId)));
    }
    /** Judge one submission. Always resolves with the final payload (never throws for user errors). */
    async judge(info, progress, opt = {}) {
        const sid = Number(info.solution_id);
        const base = {
            solution_id: sid, state: 2 /* Verdict.COMPILING */, finish: 0, judger: this.cfg.judgerName,
            judgerId: opt.judgerId || `v2-${sid}`, time: 0, memory: 0, pass_point: 0, pass_rate: 0,
            total_point: 0, compile_info: "", test_run_result: "", sim: 0, sim_s_id: 0, wid: 0,
            user_id: info.user_id, problem_id: Number(info.problem_id)
        };
        const cached = [];
        try {
            const lang = this.language(Number(info.language));
            if (!lang) {
                return { ...base, state: 11 /* Verdict.COMPILE_ERROR */, finish: 1, compile_info: `Language ${info.language} is not supported by this judge.` };
            }
            progress({ ...base });
            // ---- limits (same rules as the legacy judge_client) ----
            let timeSec = Number(info.time_limit);
            let memMB = Math.floor(Number(info.memory_limit));
            if (lang.bonus) {
                timeSec = timeSec * this.cfg.timeBonus + this.cfg.timeBonus;
                memMB = memMB + this.cfg.memoryBonus;
            }
            if (!(timeSec > 0) || timeSec > 300)
                timeSec = 300;
            if (!(memMB >= 1) || memMB > 1024)
                memMB = 1024;
            // ---- compile ----
            const compiled = await this.compile(lang, info.source);
            Object.values(compiled.fileIds).forEach(id => cached.push(id));
            if (!compiled.ok) {
                return { ...base, state: 11 /* Verdict.COMPILE_ERROR */, finish: 1, compile_info: compiled.message };
            }
            const pid = Number(info.problem_id);
            if (pid <= 0 || info.test_run) {
                return await this.testRun(base, lang, info, compiled.fileIds, timeSec, memMB, progress);
            }
            const dataDir = this.dataDir(pid);
            const cases = listTestCases(dataDir);
            const total = cases.length;
            progress({ ...base, state: 3 /* Verdict.RUNNING */, total_point: total });
            const checker = this.findChecker(dataDir, info);
            const interactor = fs_1.default.existsSync(path_1.default.join(dataDir, "interactor")) ? path_1.default.join(dataDir, "interactor") : null;
            let final = 4 /* Verdict.ACCEPT */;
            let pe = false;
            let passPoint = 0;
            let maxTime = 0;
            let maxMem = 0;
            let tle = false;
            const diffs = [];
            let reInfo = "";
            for (const tc of cases) {
                const r = interactor
                    ? await this.runInteractive(lang, info, compiled.fileIds, tc, timeSec, memMB, interactor)
                    : await this.runCase(lang, info, compiled.fileIds, tc, timeSec, memMB, checker);
                maxTime = Math.max(maxTime, r.timeMs);
                maxMem = Math.max(maxMem, r.memKB);
                if (r.verdict === 7 /* Verdict.TIME_LIMIT_EXCEEDED */)
                    tle = true;
                if (r.verdict === 5 /* Verdict.PRESENTATION_ERROR */) {
                    pe = true;
                }
                if (r.verdict === 4 /* Verdict.ACCEPT */ || r.verdict === 5 /* Verdict.PRESENTATION_ERROR */) {
                    passPoint++;
                }
                else if (r.verdict > final) {
                    final = r.verdict;
                }
                if (r.diff)
                    diffs.push(r.diff);
                if (r.verdict === 10 /* Verdict.RUNTIME_ERROR */ && !reInfo)
                    reInfo = r.message;
                if (r.verdict === 16 /* Verdict.SYSTEM_ERROR */) {
                    throw new Error(`system error on ${tc.name}: ${r.message}`);
                }
                progress({ ...base, state: 3 /* Verdict.RUNNING */, total_point: total, pass_point: passPoint,
                    pass_rate: total ? passPoint / total : 0, time: Math.min(maxTime, timeSec * 1000),
                    memory: Math.min(maxMem, memMB * 1024) });
            }
            if (final === 4 /* Verdict.ACCEPT */ && pe)
                final = 5 /* Verdict.PRESENTATION_ERROR */;
            let runtimeInfo = "";
            if (final === 10 /* Verdict.RUNTIME_ERROR */)
                runtimeInfo = reInfo;
            if (final === 6 /* Verdict.WRONG_ANSWER */ || final === 5 /* Verdict.PRESENTATION_ERROR */)
                runtimeInfo = diffs.join("");
            const payload = {
                ...base, state: final, finish: 1, total_point: total, pass_point: passPoint,
                pass_rate: total ? passPoint / total : 0,
                time: tle ? Math.round(timeSec * 1000) : Math.min(maxTime, Math.round(timeSec * 1000)),
                memory: Math.min(maxMem, memMB * 1024),
                runtime_info: truncate(runtimeInfo, this.cfg.runtimeInfoMax)
            };
            if (final === 4 /* Verdict.ACCEPT */ && this.cfg.simEnable && !opt.noSim && !opt.readOnly && this.simEnabledFor(Number(info.language))) {
                const s = await this.sim(info, lang, Boolean(opt.admin)).catch(() => ({ sim: 0, simId: 0 }));
                payload.sim = s.sim;
                payload.sim_s_id = s.simId;
            }
            return payload;
        }
        catch (e) {
            return { ...base, state: 16 /* Verdict.SYSTEM_ERROR */, finish: 1, runtime_info: `judge v2 internal error: ${e && e.message || e}` };
        }
        finally {
            for (const id of cached) {
                this.gj.deleteFile(id);
            }
        }
    }
    simEnabledFor(langId) {
        // legacy enableSim(): C/C++ family (C11 subclasses except Go/FreeBASIC) and Java family
        return [0, 1, 2, 10, 13, 14, 19, 20, 21, 28, 29, 30, 3, 23, 24, 27, 22].includes(langId);
    }
    findChecker(dataDir, info) {
        const spjFlag = String(info.spj) === "1" || info.spj === true || String(info.spj) === "true";
        if (!spjFlag)
            return null;
        if (fs_1.default.existsSync(path_1.default.join(dataDir, "spj")))
            return ["spj"];
        if (fs_1.default.existsSync(path_1.default.join(dataDir, "spj.js")))
            return ["spj.js"];
        if (fs_1.default.existsSync(path_1.default.join(dataDir, "spj.py")))
            return ["spj.py"];
        return []; // legacy: spj flag without checker => accepted
    }
    async compile(lang, source) {
        if (!lang.compile) {
            return { ok: true, message: "", fileIds: {} };
        }
        const cpuSec = lang.compileCpuSec || 10;
        const cmd = {
            args: lang.compile,
            env: ["PATH=/usr/local/bin:/usr/bin:/bin", "HOME=/tmp", "LANG=C.UTF-8", ...(lang.compileEnv || [])],
            files: [{ content: "" }, { name: "stdout", max: COMPILE_OUT_MAX }, { name: "stderr", max: COMPILE_OUT_MAX }],
            cpuLimit: cpuSec * 1000 * gojudge_1.MS,
            clockLimit: (cpuSec * 2 + 10) * 1000 * gojudge_1.MS,
            memoryLimit: 2048 * gojudge_1.MB,
            procLimit: 128,
            copyIn: { [lang.src]: { content: source } },
            copyOut: ["stdout", "stderr"],
            copyOutCached: lang.artifacts.filter(a => !a.includes("*"))
        };
        // globs (Java *.class): compile, then list them with a second call is overkill; package classes into a jar instead
        const javaClasses = lang.artifacts.includes("*.class");
        if (javaClasses) {
            cmd.args = ["/bin/sh", "-c", `${lang.compile.map(shellQuote).join(" ")} && /usr/bin/jar cf main.jar *.class`];
            cmd.copyOutCached = ["main.jar"];
        }
        const [r] = await this.gj.run([cmd]);
        const out = ((r.files && r.files.stderr) || "") + ((r.files && r.files.stdout) || "");
        if (r.status !== "Accepted") {
            let msg = out;
            if (r.status === "Time Limit Exceeded")
                msg += "\nCompile time limit exceeded.";
            if (r.status === "Memory Limit Exceeded")
                msg += "\nCompile memory limit exceeded.";
            if (r.status === "Internal Error")
                throw new Error(`compile internal error: ${r.error}`);
            return { ok: false, message: msg.trim() ? msg : `Compile failed (${r.status} ${r.exitStatus})`, fileIds: r.fileIds || {} };
        }
        if (r.fileError && r.fileError.length) {
            return { ok: false, message: (out + "\n" + r.fileError.map(f => `${f.name}: ${f.type}`).join("\n")).trim(), fileIds: r.fileIds || {} };
        }
        return { ok: true, message: out, fileIds: r.fileIds || {} };
    }
    /** files available in the run sandbox: compiled artifacts (+ source for interpreted languages) */
    runCopyIn(lang, info, fileIds) {
        const copyIn = {};
        for (const [name, id] of Object.entries(fileIds)) {
            copyIn[name] = { fileId: id };
        }
        if (lang.interpreted || !lang.compile) {
            copyIn[lang.src] = { content: info.source };
        }
        return copyIn;
    }
    runArgs(lang, memMB, fileIds) {
        let args = lang.run.map(a => a.replace(/\{mem\}/g, String(memMB)));
        if (fileIds["main.jar"]) {
            // java classes were packaged as main.jar at compile time
            const i = args.indexOf("Main");
            if (i > 0)
                args = [...args.slice(0, i), "-cp", "main.jar", ...args.slice(i)];
        }
        return args;
    }
    runLimits(lang, timeSec, memMB) {
        const cpu = Math.round(timeSec * 1000) * gojudge_1.MS;
        const memBytes = (lang.jvm ? memMB + 256 : memMB + 16) * gojudge_1.MB;
        return {
            cpuLimit: cpu,
            clockLimit: cpu * 3 + 1000 * gojudge_1.MS,
            memoryLimit: memBytes,
            stackLimit: Math.max(memBytes, 128 * gojudge_1.MB), // legacy judge: RLIMIT_STACK 128MB
            procLimit: lang.procLimit || 32
        };
    }
    classify(r, lang, memMB) {
        const stderr = (r.files && r.files.stderr) || "";
        switch (r.status) {
            case "Accepted":
                return null;
            case "Time Limit Exceeded":
                return { verdict: 7 /* Verdict.TIME_LIMIT_EXCEEDED */, message: "" };
            case "Memory Limit Exceeded":
                return { verdict: 8 /* Verdict.MEMORY_LIMIT_EXCEEDED */, message: "" };
            case "Output Limit Exceeded":
                return { verdict: 9 /* Verdict.OUTPUT_LIMIT_EXCEEDED */, message: "" };
            case "Nonzero Exit Status":
                if (lang.jvm && /OutOfMemoryError/.test(stderr))
                    return { verdict: 8 /* Verdict.MEMORY_LIMIT_EXCEEDED */, message: "" };
                // legacy judge ignored the exit code of native programs (C/C++ `return 1;` still gets judged by output);
                // for interpreters / JVM / Mono a nonzero exit means an uncaught exception -> runtime error
                if (!lang.interpreted && !lang.jvm && !lang.run[0].includes("mono"))
                    return null;
                if (/MemoryError|std::bad_alloc|out of memory/i.test(stderr) && r.memory >= memMB * gojudge_1.MB * 0.9) {
                    return { verdict: 8 /* Verdict.MEMORY_LIMIT_EXCEEDED */, message: "" };
                }
                return { verdict: 10 /* Verdict.RUNTIME_ERROR */, message: `Runtime Error: exit code ${r.exitStatus}\n${stderr}`.trim() + "\n" };
            case "Signalled":
                return { verdict: 10 /* Verdict.RUNTIME_ERROR */, message: `Runtime Error:${signalName(r.exitStatus)}\n${stderr}`.trim() + "\n" };
            default:
                return { verdict: 16 /* Verdict.SYSTEM_ERROR */, message: `${r.status}: ${r.error || ""}` };
        }
    }
    async runCase(lang, info, fileIds, tc, timeSec, memMB, checker) {
        const ansSize = fs_1.default.statSync(tc.answer).size;
        const outMax = ansSize * 2 + 1024;
        const cmd = {
            args: this.runArgs(lang, memMB, fileIds),
            env: ["PATH=/usr/local/bin:/usr/bin:/bin", "HOME=/w", "LANG=C.UTF-8", "ONLINE_JUDGE=1", ...(lang.runEnv || [])],
            files: [{ src: tc.input }, { name: "stdout", max: outMax }, { name: "stderr", max: STDERR_MAX }],
            ...this.runLimits(lang, timeSec, memMB),
            copyIn: this.runCopyIn(lang, info, fileIds),
            copyOut: ["stdout", "stderr"]
        };
        const [r] = await this.gj.run([cmd]);
        const timeMs = Math.round(r.time / gojudge_1.MS);
        const memKB = Math.round(r.memory / 1024);
        const produced = ((r.files && r.files.stdout) || "").length;
        if (produced >= outMax && r.status !== "Accepted" && r.status !== "Memory Limit Exceeded") {
            // stdout collector full: the program kept printing (go-judge may report it as TLE once the pipe blocks)
            return { verdict: 9 /* Verdict.OUTPUT_LIMIT_EXCEEDED */, message: "", timeMs, memKB, diff: "" };
        }
        const bad = this.classify(r, lang, memMB);
        if (bad) {
            return { verdict: bad.verdict, message: bad.message, timeMs, memKB, diff: "" };
        }
        const userOut = (r.files && r.files.stdout) || "";
        const cmp = checker === null
            ? await this.compareText(tc, userOut)
            : await this.special(checker, tc, userOut, info);
        return { verdict: cmp.verdict, message: "", timeMs, memKB, diff: cmp.diff };
    }
    /** Legacy ZOJ text checker (cupoj-compare); same AC/PE/WA rules and diff report as the old judge */
    async compareText(tc, userOut) {
        const dir = fs_1.default.mkdtempSync(path_1.default.join(this.cfg.tmpDir, "cmp-"));
        try {
            fs_1.default.symlinkSync(tc.input, path_1.default.join(dir, "data.in"));
            fs_1.default.writeFileSync(path_1.default.join(dir, "user.out"), userOut);
            const { stdout } = await execFileAsync(this.cfg.compareBin, [tc.answer, "user.out", this.cfg.fullDiff ? "1" : "0"], { cwd: dir, timeout: 20000, maxBuffer: 1 << 20 });
            const v = parseInt(stdout.trim(), 10);
            let diff = "";
            if (v === 6 /* Verdict.WRONG_ANSWER */ || v === 5 /* Verdict.PRESENTATION_ERROR */) {
                try {
                    diff = fs_1.default.readFileSync(path_1.default.join(dir, "diff.out"), "utf8");
                }
                catch {
                    diff = "";
                }
            }
            return { verdict: Number.isFinite(v) ? v : 16 /* Verdict.SYSTEM_ERROR */, diff };
        }
        finally {
            fs_1.default.rmSync(dir, { recursive: true, force: true });
        }
    }
    /** Special judge: `spj in ans user code`, run inside the sandbox. exit 0 AC, <4 WA, >=4 used as verdict */
    async special(checker, tc, userOut, info) {
        if (checker.length === 0) {
            return { verdict: 4 /* Verdict.ACCEPT */, diff: "" };
        }
        const dataDir = path_1.default.dirname(tc.input);
        const file = checker[0];
        const runner = file === "spj" ? ["./spj"] : file === "spj.js" ? ["/usr/local/lib/judge-node", "spj.js"] : ["/usr/bin/python3", "spj.py"];
        const cmd = {
            args: [...runner, "in.txt", "ans.txt", "user.txt", "code.txt"],
            env: ["PATH=/usr/local/bin:/usr/bin:/bin", "HOME=/w", "LANG=C.UTF-8"],
            files: [{ content: "" }, { name: "stdout", max: 64 * 1024 }, { name: "stderr", max: 64 * 1024 }],
            cpuLimit: 5000 * gojudge_1.MS,
            clockLimit: 10000 * gojudge_1.MS,
            memoryLimit: 2048 * gojudge_1.MB,
            procLimit: 64,
            copyIn: {
                [file]: { src: path_1.default.join(dataDir, file) },
                "in.txt": { src: tc.input },
                "ans.txt": { src: tc.answer },
                "user.txt": { content: userOut },
                "code.txt": { content: info.source }
            },
            copyOut: ["stdout", "stderr"]
        };
        const [r] = await this.gj.run([cmd]);
        const out = (r.files && r.files.stdout) || "";
        if (r.status === "Accepted")
            return { verdict: 4 /* Verdict.ACCEPT */, diff: out };
        if (r.status === "Nonzero Exit Status") {
            const code = r.exitStatus;
            const v = code < 4 /* Verdict.ACCEPT */ ? 6 /* Verdict.WRONG_ANSWER */ : code;
            return { verdict: v, diff: out };
        }
        if (r.status === "Internal Error")
            throw new Error(`spj internal error: ${r.error}`);
        return { verdict: 6 /* Verdict.WRONG_ANSWER */, diff: out + `\n[spj ${r.status}]\n` };
    }
    /** Interactive problem: user program <-> interactor (testlib: interactor <in> <tout> <ans>) */
    async runInteractive(lang, info, fileIds, tc, timeSec, memMB, interactor) {
        const limits = this.runLimits(lang, timeSec, memMB);
        const user = {
            args: this.runArgs(lang, memMB, fileIds),
            env: ["PATH=/usr/local/bin:/usr/bin:/bin", "HOME=/w", "LANG=C.UTF-8", "ONLINE_JUDGE=1", ...(lang.runEnv || [])],
            files: [null, null, { name: "stderr", max: STDERR_MAX }],
            ...limits,
            copyIn: this.runCopyIn(lang, info, fileIds),
            copyOut: ["stderr"]
        };
        const inter = {
            args: ["./interactor", "in.txt", "tout.txt", "ans.txt"],
            env: ["PATH=/usr/local/bin:/usr/bin:/bin"],
            files: [null, null, { name: "stderr", max: 64 * 1024 }],
            cpuLimit: limits.cpuLimit * 2 + 2000 * gojudge_1.MS,
            clockLimit: limits.clockLimit * 2 + 2000 * gojudge_1.MS,
            memoryLimit: 1024 * gojudge_1.MB,
            procLimit: 16,
            copyIn: { "interactor": { src: interactor }, "in.txt": { src: tc.input }, "ans.txt": { src: tc.answer } },
            copyOut: ["stderr"]
        };
        const [ru, ri] = await this.gj.run([user, inter], [
            { in: { index: 0, fd: 1 }, out: { index: 1, fd: 0 } },
            { in: { index: 1, fd: 1 }, out: { index: 0, fd: 0 } }
        ]);
        const timeMs = Math.round(ru.time / gojudge_1.MS);
        const memKB = Math.round(ru.memory / 1024);
        const bad = this.classify(ru, lang, memMB);
        const interMsg = (ri.files && ri.files.stderr) || "";
        // testlib exit codes: 0 ok, 1 wa, 2 pe, 3 fail, 7 points
        if (ri.status === "Nonzero Exit Status" && (ri.exitStatus === 1 || ri.exitStatus === 2)) {
            // user error found by the interactor wins over a user crash caused by the closed pipe
            return { verdict: ri.exitStatus === 2 ? 5 /* Verdict.PRESENTATION_ERROR */ : 6 /* Verdict.WRONG_ANSWER */, message: "", timeMs, memKB, diff: `========[${tc.name}]=========\n${interMsg}\n` };
        }
        if (bad)
            return { verdict: bad.verdict, message: bad.message, timeMs, memKB, diff: "" };
        if (ri.status === "Accepted")
            return { verdict: 4 /* Verdict.ACCEPT */, message: "", timeMs, memKB, diff: "" };
        if (ri.status === "Time Limit Exceeded")
            return { verdict: 7 /* Verdict.TIME_LIMIT_EXCEEDED */, message: "", timeMs, memKB, diff: "" };
        return { verdict: 16 /* Verdict.SYSTEM_ERROR */, message: `interactor ${ri.status} ${ri.exitStatus}: ${interMsg}`, timeMs, memKB, diff: "" };
    }
    async testRun(base, lang, info, fileIds, timeSec, memMB, progress) {
        progress({ ...base, state: 3 /* Verdict.RUNNING */ });
        const cmd = {
            args: this.runArgs(lang, memMB, fileIds),
            env: ["PATH=/usr/local/bin:/usr/bin:/bin", "HOME=/w", "LANG=C.UTF-8", "ONLINE_JUDGE=1", ...(lang.runEnv || [])],
            files: [{ content: info.custom_input || "" }, { name: "stdout", max: 16 * gojudge_1.MB }, { name: "stderr", max: STDERR_MAX }],
            ...this.runLimits(lang, timeSec, memMB),
            copyIn: this.runCopyIn(lang, info, fileIds),
            copyOut: ["stdout", "stderr"]
        };
        const [r] = await this.gj.run([cmd]);
        const bad = this.classify(r, lang, memMB);
        let out;
        let timeMs = Math.round(r.time / gojudge_1.MS);
        if (!bad) {
            out = (r.files && r.files.stdout) || "";
        }
        else if (bad.verdict === 7 /* Verdict.TIME_LIMIT_EXCEEDED */) {
            out = "Time Limit Exceeded.Kill Process.\n";
            timeMs = Math.round(timeSec * 1000);
        }
        else if (bad.verdict === 8 /* Verdict.MEMORY_LIMIT_EXCEEDED */) {
            out = "Memory Limit Exceeded.Kill Process.\n";
        }
        else if (bad.verdict === 16 /* Verdict.SYSTEM_ERROR */) {
            throw new Error(bad.message);
        }
        else {
            out = "Runtime Error. Kill Process.\n" + ((r.files && r.files.stderr) || "");
        }
        if (out.length > 4096) {
            out = out.slice(0, 4096) + `\n......Omit ${out.length - 4096} characters.`;
        }
        if (bad && bad.verdict === 7 /* Verdict.TIME_LIMIT_EXCEEDED */) {
            out += "\n测试运行中发生运行超时，程序被强制停止";
        }
        return { ...base, state: 13 /* Verdict.TEST_RUN */, finish: 1, time: timeMs, memory: Math.round(r.memory / 1024), test_run_result: out };
    }
    /** Legacy plagiarism check: /usr/bin/sim.sh against data/<pid>/ac, then archive this AC source there */
    async sim(info, lang, admin) {
        const pid = Math.abs(Number(info.problem_id));
        const ext = lang.src.split(".").pop();
        const work = fs_1.default.mkdtempSync(path_1.default.join(this.cfg.ojHome, "kcv2-sim-"));
        try {
            const src = `Main.${ext}`;
            fs_1.default.writeFileSync(path_1.default.join(work, src), info.source);
            let sim = 0;
            let simId = 0;
            if (!admin && fs_1.default.existsSync(this.cfg.simScript)) {
                // sim.sh looks at ../data/<pid>/ac relative to cwd, writes "<sim> <file>" to ./sim when found
                await execFileAsync("/bin/bash", [this.cfg.simScript, src, String(pid)], { cwd: work, timeout: 30000 }).catch(() => undefined);
                const f = path_1.default.join(work, "sim");
                if (fs_1.default.existsSync(f)) {
                    const [a, b] = fs_1.default.readFileSync(f, "utf8").trim().split(/\s+/);
                    sim = parseInt(a, 10) || 0;
                    simId = parseInt(b, 10) || 0;
                }
            }
            if (!sim) {
                const acDir = path_1.default.join(this.cfg.ojHome, "data", String(pid), "ac");
                fs_1.default.mkdirSync(acDir, { recursive: true });
                const dst = path_1.default.join(acDir, `${info.solution_id}.${ext}`);
                fs_1.default.writeFileSync(dst, info.source);
                if (ext === "c" || ext === "cc") {
                    const other = path_1.default.join(acDir, `${info.solution_id}.${ext === "c" ? "cc" : "c"}`);
                    try {
                        fs_1.default.symlinkSync(dst, other);
                    }
                    catch { /* exists */ }
                }
            }
            return { sim, simId };
        }
        finally {
            fs_1.default.rmSync(work, { recursive: true, force: true });
        }
    }
}
exports.Judge = Judge;
function shellQuote(s) {
    return /^[A-Za-z0-9_\/.=+-]+$/.test(s) ? s : `'${s.replace(/'/g, `'\\''`)}'`;
}
//# sourceMappingURL=judge.js.map