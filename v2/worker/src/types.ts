/** Result codes shared with the website (solution.result). */
export const enum Verdict {
	WAITING = 0,
	REJUDGE = 1,
	COMPILING = 2,
	RUNNING = 3,
	ACCEPT = 4,
	PRESENTATION_ERROR = 5,
	WRONG_ANSWER = 6,
	TIME_LIMIT_EXCEEDED = 7,
	MEMORY_LIMIT_EXCEEDED = 8,
	OUTPUT_LIMIT_EXCEEDED = 9,
	RUNTIME_ERROR = 10,
	COMPILE_ERROR = 11,
	TEST_RUN = 13,
	SYSTEM_ERROR = 16
}

/** What the backend sends with a `submission` event (JudgeManager.buildSubmissionInfo). */
export interface SubmissionInfo {
	solution_id: number;
	source: string;
	custom_input?: string | null;
	test_run?: boolean;
	language: number;
	user_id: string;
	problem_id: number;
	spj?: boolean | number | string;
	time_limit: number;   // seconds
	memory_limit: number; // MB
	[k: string]: any;
}

/** Payload of a `judger` event (same fields the legacy judge_client sends). */
export interface JudgePayload {
	solution_id: number;
	state: number;
	finish: 0 | 1;
	judger: string;
	judgerId: string;
	time: number;          // ms
	memory: number;        // KB
	pass_point: number;
	pass_rate: number;
	total_point: number;
	compile_info: string;
	runtime_info?: string;
	test_run_result: string;
	sim: number;
	sim_s_id: number;
	wid: number;
	user_id?: string;
	problem_id?: number;
}

export interface LanguageDef {
	name: string;
	src: string;
	compile: string[] | null;
	compileEnv?: string[];
	compileCpuSec?: number;
	artifacts: string[];
	run: string[];
	runEnv?: string[];
	bonus?: boolean;
	jvm?: boolean;
	interpreted?: boolean;
	procLimit?: number;
}

export interface WorkerConfig {
	port: number;
	host: string;
	goJudgeUrl: string;
	ojHome: string;            // /home/judge
	judgerName: string;        // shown as solution.judger
	concurrency: number;
	languagesFile: string;
	compareBin: string;        // path to cupoj-compare
	tmpDir: string;
	timeBonus: number;         // java_time_bonus (2)
	memoryBonus: number;       // java_memory_bonus (128)
	fullDiff: boolean;
	simEnable: boolean;
	simScript: string;         // legacy /usr/bin/sim.sh
	maxSubmissionSeconds: number;
	runtimeInfoMax: number;
	/** if set, clients must connect with ?token=<authToken> (put it in the backend's judger.address) */
	authToken?: string;
}
