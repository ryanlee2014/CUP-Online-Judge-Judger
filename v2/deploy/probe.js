// End-to-end probe: connect to a judge server exactly like the website backend (socket.io-client 2.x),
// send one submission built from a JSON file, print every `judger` event until finish.
// usage: node probe.js <url-with-?token=> <submission.json> [socketIoClientPath]
const [url, file, clientPath] = process.argv.slice(2);
const io = require(clientPath || "/home/CUP-Online-Judge-Express/node_modules/socket.io-client");
const data = JSON.parse(require("fs").readFileSync(file, "utf8"));
const socket = io(url);
const t0 = Date.now();
const timer = setTimeout(() => { console.log("TIMEOUT"); process.exit(2); }, 120000);
socket.on("connect", () => {
	socket.emit("type", { type: "submitter" });
	socket.emit("status", {});
	socket.emit("submission", { solutionId: data.solution_id, admin: true, data, no_sim: true, priority: 1 });
});
socket.on("status", s => console.log("status free=", s.free_judger && s.free_judger.length, "version=", s.version));
socket.on("reject_judge", p => { console.log("REJECT", p); process.exit(1); });
socket.on("judger", p => {
	console.log(`${Date.now() - t0}ms state=${p.state} finish=${p.finish} pass=${p.pass_point}/${p.total_point} time=${p.time} mem=${p.memory}`
		+ (p.finish ? ` info=${JSON.stringify((p.compile_info || p.runtime_info || p.test_run_result || "").slice(0, 200))}` : ""));
	if (p.finish) { clearTimeout(timer); socket.close(); process.exit(0); }
});
socket.on("disconnect", () => console.log("disconnected"));
