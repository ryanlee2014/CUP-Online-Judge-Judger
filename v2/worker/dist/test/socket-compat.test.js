"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
/**
 * The website backend talks to this server with socket.io-client. During the 2.x -> 4.x migration both
 * client generations must be able to connect, pass the token check, submit and receive results.
 */
const node_test_1 = __importDefault(require("node:test"));
const node_assert_1 = __importDefault(require("node:assert"));
const server_1 = require("../server");
// eslint-disable-next-line @typescript-eslint/no-var-requires
const clientV2 = require("socket.io-client-v2");
// eslint-disable-next-line @typescript-eslint/no-var-requires
const clientV4 = require("socket.io-client").io;
const stubJudge = {
    async judge(info, emit) {
        emit({ solution_id: info.solution_id, state: 3, finish: 0 });
        return { solution_id: info.solution_id, state: 4, finish: 1, pass_point: 1, total_point: 1, time: 1, memory: 1 };
    }
};
function startServer(extra = {}) {
    const cfg = {
        port: 0, host: "127.0.0.1", ojHome: "/nonexistent", judgerName: "TEST", concurrency: 2,
        maxSubmissionSeconds: 30, authToken: "secret", ...extra
    };
    const server = new server_1.JudgeServer(cfg, stubJudge).start();
    return new Promise(resolve => server.on("listening", () => {
        const port = server.address().port;
        resolve({ url: `http://127.0.0.1:${port}`, close: () => server.close() });
    }));
}
function roundTrip(connect, url) {
    return new Promise((resolve, reject) => {
        const events = [];
        const socket = connect(url);
        const timer = setTimeout(() => { socket.close(); reject(new Error("timeout")); }, 5000);
        socket.on("connect", () => {
            socket.emit("status", {});
            // problem_id 0 = test run: no data directory needed
            socket.emit("submission", { solutionId: 42, admin: false, no_sim: true, priority: 1, data: { solution_id: 42, problem_id: 0 } });
        });
        socket.on("judger", (p) => {
            events.push(p);
            if (p.finish) {
                clearTimeout(timer);
                socket.close();
                resolve(events);
            }
        });
    });
}
for (const [name, connect] of [
    ["socket.io-client 2.x", (url) => clientV2(url, { forceNew: true })],
    ["socket.io-client 4.x", (url) => clientV4(url, { forceNew: true })]
]) {
    (0, node_test_1.default)(`${name}: token in the URL, submit, progress and final result`, async () => {
        const srv = await startServer();
        try {
            const events = await roundTrip(connect, `${srv.url}?token=secret`);
            node_assert_1.default.deepStrictEqual(events.map(e => [e.state, e.finish]), [[3, 0], [4, 1]]);
            node_assert_1.default.strictEqual(events[1].solution_id, 42);
        }
        finally {
            srv.close();
        }
    });
    (0, node_test_1.default)(`${name}: wrong token is disconnected`, async () => {
        const srv = await startServer();
        try {
            await node_assert_1.default.rejects(roundTrip(connect, `${srv.url}?token=wrong`), /timeout/);
        }
        finally {
            srv.close();
        }
    });
}
(0, node_test_1.default)("allowEIO3=false: 2.x clients are refused, 4.x still works", async () => {
    const srv = await startServer({ allowEIO3: false });
    try {
        await node_assert_1.default.rejects(roundTrip((u) => clientV2(u, { forceNew: true, reconnection: false }), `${srv.url}?token=secret`), /timeout/);
        const events = await roundTrip((u) => clientV4(u, { forceNew: true }), `${srv.url}?token=secret`);
        node_assert_1.default.strictEqual(events[events.length - 1].state, 4);
    }
    finally {
        srv.close();
    }
});
//# sourceMappingURL=socket-compat.test.js.map