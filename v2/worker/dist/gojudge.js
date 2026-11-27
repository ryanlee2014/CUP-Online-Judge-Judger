"use strict";
/**
 * Minimal client for the go-judge REST API (https://github.com/criyle/go-judge).
 * Units on the wire: time in ns, memory in bytes.
 */
Object.defineProperty(exports, "__esModule", { value: true });
exports.MB = exports.MS = exports.GoJudgeClient = void 0;
class GoJudgeClient {
    baseUrl;
    constructor(baseUrl) {
        this.baseUrl = baseUrl;
    }
    async run(cmd, pipeMapping) {
        const res = await fetch(`${this.baseUrl}/run`, {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ cmd, pipeMapping })
        });
        if (!res.ok) {
            throw new Error(`go-judge /run HTTP ${res.status}: ${(await res.text()).slice(0, 500)}`);
        }
        return await res.json();
    }
    async deleteFile(fileId) {
        try {
            await fetch(`${this.baseUrl}/file/${encodeURIComponent(fileId)}`, { method: "DELETE" });
        }
        catch {
            // cache files also expire by -file-timeout
        }
    }
    async version() {
        const res = await fetch(`${this.baseUrl}/version`);
        return await res.json();
    }
}
exports.GoJudgeClient = GoJudgeClient;
exports.MS = 1_000_000;
exports.MB = 1024 * 1024;
//# sourceMappingURL=gojudge.js.map