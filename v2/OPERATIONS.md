# 判题机 v2 运维手册（.28）

## 组成

| 进程 | pm2 名称（root） | 位置 | 端口 |
|---|---|---|---|
| go-judge 沙箱 | `kc-go-judge` | `/opt/kc-gojudge/`（`run-go-judge.sh` 启动，设 swappiness=0） | 127.0.0.1:5050 |
| 判题服务 | `kc-judge-v2` | `/opt/kc-judge-v2/app/`（`config.json` 权限 600，含接入口令） | 0.0.0.0:5111 |
| 旧判题守护进程（回退用） | `CUP Online Judge judger daemon service` | `/home/daemon` | 5110 / 5100 |

健康检查：`curl -s 127.0.0.1:5111/healthz`（judging / waiting / free），`curl -s 127.0.0.1:5050/version`。

## 切换 / 回退

后端 `/home/CUP-Online-Judge-Express/config.json` 的 `judger.address`：

- 新判题机：`http://localhost:5111?token=<config.json 里的 authToken>`
- 旧判题机：`http://localhost:5110`

改完 `pm2 restart 0 1`（后端 http + websocket）。旧守护进程一直保留运行，回退只需改回地址再重启后端。

## 配置

- `languages.json`：语言表（编译/运行命令、产物、时间内存加成）。改完 `pm2 restart kc-judge-v2`。
- `config.json`：`concurrency`（并发判题数，默认 24）、`judgerName`（写入 solution.judger）、`authToken`。
- 题目数据仍是 `/home/judge/data/<pid>`：`*.in` + 同名 `.out`；SPJ 为 `spj` / `spj.js` / `spj.py`；交互题放 `interactor`（testlib 约定：`interactor <in> <tout> <ans>`，退出码 0 AC / 1 WA / 2 PE）。

## 验证工具

- `node dist/replay.js --db /home/CUP-Online-Judge-Express/config.json --out x.jsonl --sql "<条件>" --limit N`：只读回放历史提交，与库里结论对比。
- `bash oldjudge-replay.sh ids.txt out.tsv`：同机用旧判题机重判，做新旧同数据对照。
- `node probe.js 'http://127.0.0.1:5111?token=...' sub.json`：按网站后端的方式发一份提交，打印全部进度事件。

## acm（.16，CentOS 7.9）

- 2026-11-26 起 acm 也用判题机 v2。.16 内核 3.10 且 `user.max_user_namespaces=0`，官方 go-judge 起不来；这里用打了 `deploy/go-judge-patch/go-sandbox-v0.14.6-no-userns.patch` 的构建（`/opt/kc-gojudge/go-judge-kc`），`KC_NO_USERNS=1` 启动：不建用户命名空间，其余 mount/pid/net/ipc/uts 隔离照旧；选手程序以 uid/gid 61536（宿主无此账号）运行，无 capability、无附加组、seccomp 照旧。系统安全设置没改。
- pm2（`PM2_HOME=/root/.pm2-n26 /opt/node26/bin/pm2`）：`kc-go-judge`（`run-go-judge-16.sh`，127.0.0.1:5050，挂载 `mount-cupoj16-nouserns.yaml`）、`kc-judge-v2`（127.0.0.1:5111，`judgerName` RATH，并发 6）。已 `pm2 save`，随 `kc-oj-pm2.service` 开机自启。
- 语言表 `worker/languages.cupoj16.json`：路径按 .16 旧判题机；PHP 用 `-n` + 白名单扩展（.16 的 php.ini 会加载 redis 会话等）；Java 6/7 用 `MaxPermSize`；Go 编译 CPU 放宽到 60 秒。
- 后端 `/home/CUP-Online-Judge-Express-n26/config.json` 的 `judger.address` 指向 5111；旧守护进程（5110）保留。回退：`bash /root/kc-backup-jv2-20261126-1455/rollback.sh`。

## 从仓库部署 worker

`v2/worker/node_modules` 不进仓库。部署前在 `v2/worker` 下执行 `npm ci --omit=dev`（Node 20+；`dist/` 已随仓库提交，改了 `src/` 再 `npx tsc` 重新生成），然后把 `dist/ node_modules/ package.json` 与对应机器的语言表（改名为 `languages.json`）复制到 `/opt/kc-judge-v2/app/`。
