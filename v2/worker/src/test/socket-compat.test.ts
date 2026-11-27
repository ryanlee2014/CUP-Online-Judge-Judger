/**
 * The website backend talks to this server with socket.io-client. During the 2.x -> 4.x migration both
 * client generations must be able to connect, pass the token check, submit and receive results.
 */
import test from "node:test";
import assert from "node:assert";
import { AddressInfo } from "net";
import { JudgeServer } from "../server";
import { WorkerConfig } from "../types";
// eslint-disable-next-line @typescript-eslint/no-var-requires
const clientV2 = require("socket.io-client-v2");
// eslint-disable-next-line @typescript-eslint/no-var-requires
const clientV4 = require("socket.io-client").io;

const stubJudge: any = {
	async judge(info: any, emit: (p: any) => void) {
		emit({ solution_id: info.solution_id, state: 3, finish: 0 });
		return { solution_id: info.solution_id, state: 4, finish: 1, pass_point: 1, total_point: 1, time: 1, memory: 1 };
	}
};

function startServer(extra: Partial<WorkerConfig> = {}) {
	const cfg = {
		port: 0, host: "127.0.0.1", ojHome: "/nonexistent", judgerName: "TEST", concurrency: 2,
		maxSubmissionSeconds: 30, authToken: "secret", ...extra
	} as WorkerConfig;
	const server = new JudgeServer(cfg, stubJudge).start();
	return new Promise<{ url: string, close: () => void }>(resolve => server.on("listening", () => {
		const port = (server.address() as AddressInfo).port;
		resolve({ url: `http://127.0.0.1:${port}`, close: () => server.close() });
	}));
}

function roundTrip(connect: (url: string) => any, url: string): Promise<any[]> {
	return new Promise((resolve, reject) => {
		const events: any[] = [];
		const socket = connect(url);
		const timer = setTimeout(() => { socket.close(); reject(new Error("timeout")); }, 5000);
		socket.on("connect", () => {
			socket.emit("status", {});
			// problem_id 0 = test run: no data directory needed
			socket.emit("submission", { solutionId: 42, admin: false, no_sim: true, priority: 1, data: { solution_id: 42, problem_id: 0 } });
		});
		socket.on("judger", (p: any) => {
			events.push(p);
			if (p.finish) { clearTimeout(timer); socket.close(); resolve(events); }
		});
	});
}

for (const [name, connect] of [
	["socket.io-client 2.x", (url: string) => clientV2(url, { forceNew: true })],
	["socket.io-client 4.x", (url: string) => clientV4(url, { forceNew: true })]
] as const) {
	test(`${name}: token in the URL, submit, progress and final result`, async () => {
		const srv = await startServer();
		try {
			const events = await roundTrip(connect, `${srv.url}?token=secret`);
			assert.deepStrictEqual(events.map(e => [e.state, e.finish]), [[3, 0], [4, 1]]);
			assert.strictEqual(events[1].solution_id, 42);
		} finally {
			srv.close();
		}
	});

	test(`${name}: wrong token is disconnected`, async () => {
		const srv = await startServer();
		try {
			await assert.rejects(roundTrip(connect, `${srv.url}?token=wrong`), /timeout/);
		} finally {
			srv.close();
		}
	});
}

test("allowEIO3=false: 2.x clients are refused, 4.x still works", async () => {
	const srv = await startServer({ allowEIO3: false });
	try {
		await assert.rejects(roundTrip((u: string) => clientV2(u, { forceNew: true, reconnection: false }), `${srv.url}?token=secret`), /timeout/);
		const events = await roundTrip((u: string) => clientV4(u, { forceNew: true }), `${srv.url}?token=secret`);
		assert.strictEqual(events[events.length - 1].state, 4);
	} finally {
		srv.close();
	}
});
