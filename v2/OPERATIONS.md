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
