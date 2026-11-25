/**
 * Minimal client for the go-judge REST API (https://github.com/criyle/go-judge).
 * Units on the wire: time in ns, memory in bytes.
 */

export type GoFile =
	| { src: string }
	| { content: string }
	| { fileId: string }
	| { name: string; max: number; pipe?: boolean }
	| null;

export interface GoCmd {
	args: string[];
	env?: string[];
	files?: GoFile[];
	tty?: boolean;
	cpuLimit?: number;
	clockLimit?: number;
	memoryLimit?: number;
	stackLimit?: number;
	procLimit?: number;
	cpuRateLimit?: number;
	strictMemoryLimit?: boolean;
	addressSpaceLimit?: boolean;
	copyIn?: Record<string, GoFile>;
	copyOut?: string[];
	copyOutCached?: string[];
	copyOutMax?: number;
}

export interface PipeIndex { index: number; fd: number }
export interface PipeMap { in: PipeIndex; out: PipeIndex; proxy?: boolean; name?: string; max?: number }

export type GoStatus =
	| "Accepted"
	| "Memory Limit Exceeded"
	| "Time Limit Exceeded"
	| "Output Limit Exceeded"
	| "File Error"
	| "Nonzero Exit Status"
	| "Signalled"
	| "Internal Error";

export interface GoResult {
	status: GoStatus;
	exitStatus: number;
	error?: string;
	time: number;
	memory: number;
	runTime: number;
	files?: Record<string, string>;
	fileIds?: Record<string, string>;
	fileError?: { name: string; type: string; message?: string }[];
}

export class GoJudgeClient {
	constructor(private readonly baseUrl: string) {}

	async run(cmd: GoCmd[], pipeMapping?: PipeMap[]): Promise<GoResult[]> {
		const res = await fetch(`${this.baseUrl}/run`, {
			method: "POST",
			headers: { "Content-Type": "application/json" },
			body: JSON.stringify({ cmd, pipeMapping })
		});
		if (!res.ok) {
			throw new Error(`go-judge /run HTTP ${res.status}: ${(await res.text()).slice(0, 500)}`);
		}
		return await res.json() as GoResult[];
	}

	async deleteFile(fileId: string): Promise<void> {
		try {
			await fetch(`${this.baseUrl}/file/${encodeURIComponent(fileId)}`, { method: "DELETE" });
		} catch {
			// cache files also expire by -file-timeout
		}
	}

	async version(): Promise<any> {
		const res = await fetch(`${this.baseUrl}/version`);
		return await res.json();
	}
}

export const MS = 1_000_000;
export const MB = 1024 * 1024;
