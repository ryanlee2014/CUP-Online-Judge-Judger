/* eslint-disable no-console */
import fs from "fs";
import http from "http";
import os from "os";
import path from "path";
import { Judge } from "./judge";
import { JudgePayload, SubmissionInfo, WorkerConfig } from "./types";
// socket.io v2 server: the website backend connects with socket.io-client 2.x (same as the legacy daemon)
// eslint-disable-next-line @typescript-eslint/no-var-requires
const SocketIO = require("socket.io");

interface Task {
	solutionId: number;
	info: SubmissionInfo;
	admin: boolean;
	noSim: boolean;
	priority: number;
	socket: any;
}

/**
 * Protocol-compatible replacement of the legacy judge daemon (CUP-Online-Judge-Judge-Daemon-Service):
 *  in : `submission` {solutionId, admin, data, no_sim, priority}, `status`
 *  out: `judger` (progress + final, same payload as judge_client), `reject_judge` {reason, solutionId, problemId},
 *       `status`, `change` (free slots), `error_record` {solutionId, recordId}
 */
export class JudgeServer {
	private queue: Task[] = [];
	private inQueue = new Set<number>();
	private judging = new Set<number>();
	private freeSlots: number[];
	private latestSolutionId = 0;
	private sockets = new Map<number, any>();
	private nextSocketId = 0;
	private io: any;

	constructor(private readonly cfg: WorkerConfig, private readonly judge: Judge) {
		this.freeSlots = [...Array(cfg.concurrency).keys()].map(i => i + 1);
	}

	start() {
		const server = http.createServer((req, res) => {
			if (req.url === "/healthz") {
				res.writeHead(200, { "Content-Type": "application/json" });
				res.end(JSON.stringify({ ok: true, judging: [...this.judging], waiting: this.queue.length, free: this.freeSlots.length }));
				return;
			}
			res.writeHead(404);
			res.end();
		});
		this.io = SocketIO(server);
		this.io.on("connection", (socket: any) => this.bind(socket));
		server.listen(this.cfg.port, this.cfg.host, () => console.log(`judge v2 listening on ${this.cfg.host}:${this.cfg.port}`));
	}

	status() {
		const cpus = os.cpus();
		return {
			judging: [...this.judging],
			free_judger: this.freeSlots,
			waiting: this.queue.map(t => ({ solution_id: t.solutionId, priority: t.priority })),
			last_solution_id: this.latestSolutionId,
			oj_home: this.cfg.ojHome,
			cpu_details: cpus,
			cpu_model: cpus[0] && cpus[0].model,
			cpu_speed: cpus[0] && cpus[0].speed,
			platform: os.platform(),
			version: "judge-v2"
		};
	}

	private bind(socket: any) {
		const id = this.nextSocketId++;
		this.sockets.set(id, socket);
		socket.on("submission", (payload: any) => this.onSubmission(socket, payload));
		socket.on("status", () => socket.emit("status", this.status()));
		socket.on("disconnect", () => this.sockets.delete(id));
	}

	private broadcastChange() {
		for (const s of this.sockets.values()) {
			s.emit("change", this.freeSlots);
		}
	}

	private onSubmission(socket: any, payload: any) {
		const solutionId = parseInt(payload && payload.solutionId, 10);
		const info: SubmissionInfo = payload && payload.data;
		if (!solutionId || !info) {
			return;
		}
		if (this.judging.has(solutionId) || this.inQueue.has(solutionId)) {
			return;
		}
		const pid = Number(info.problem_id);
		if (pid > 0 && !fs.existsSync(path.join(this.cfg.ojHome, "data", String(Math.abs(pid))))) {
			console.log(`Lost data: ${pid}`);
			socket.emit("reject_judge", { reason: "No data", solutionId, problemId: pid });
			return;
		}
		console.log(`Get Submission: ${solutionId}`);
		this.latestSolutionId = Math.max(this.latestSolutionId, solutionId);
		this.queue.push({
			solutionId, info, socket,
			admin: Boolean(payload.admin), noSim: Boolean(payload.no_sim),
			priority: Number(payload.priority) || 0
		});
		this.inQueue.add(solutionId);
		// higher priority first, then newer submissions first (legacy daemon order)
		this.queue.sort((a, b) => (b.priority - a.priority) || (b.solutionId - a.solutionId));
		this.pump();
	}

	private pump() {
		while (this.freeSlots.length && this.queue.length) {
			const task = this.queue.shift() as Task;
			this.inQueue.delete(task.solutionId);
			const slot = this.freeSlots.shift() as number;
			this.judging.add(task.solutionId);
			this.broadcastChange();
			this.run(task, slot).finally(() => {
				this.judging.delete(task.solutionId);
				this.freeSlots.push(slot);
				this.broadcastChange();
				this.pump();
			});
		}
	}

	private async run(task: Task, slot: number) {
		const started = Date.now();
		const emit = (p: JudgePayload) => {
			p.wid = slot;
			task.socket.emit("judger", p);
		};
		const deadline = new Promise<JudgePayload>(resolve => setTimeout(() => resolve({
			solution_id: task.solutionId, state: 16, finish: 1, judger: this.cfg.judgerName, judgerId: `v2-${task.solutionId}`,
			time: 0, memory: 0, pass_point: 0, pass_rate: 0, total_point: 0, compile_info: "", test_run_result: "",
			sim: 0, sim_s_id: 0, wid: slot, runtime_info: "judge v2: submission took too long"
		}), this.cfg.maxSubmissionSeconds * 1000));
		try {
			const final = await Promise.race([
				this.judge.judge(task.info, emit, { admin: task.admin, noSim: task.noSim, judgerId: `v2-${task.solutionId}` }),
				deadline
			]);
			emit(final);
			console.log(`Judged ${task.solutionId} state=${final.state} pass=${final.pass_point}/${final.total_point} ${Date.now() - started}ms`);
		} catch (e: any) {
			console.error(`Judge ${task.solutionId} failed:`, e);
			task.socket.emit("error_record", { solutionId: task.solutionId, recordId: slot });
		}
	}
}
