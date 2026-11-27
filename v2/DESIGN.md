# CUP OJ 判题机 v2 设计

> 状态：设计 + 实现中（2026-11-26）。分支 `feat/judge-v2`，代码在 `v2/`。

## 1. 为什么重做

现有判题机（`judge_client` + `wsjudged` + Node 守护进程）是 2017 年 HUSTOJ 派生的 C++ 实现，问题集中在沙箱与运维：

| 问题 | 现状 |
|---|---|
| 沙箱 | ptrace 逐个系统调用放行 + 每种语言一份白名单 + chroot。新语言/新版本解释器一用新系统调用就被判成运行错误（本周修了 Ruby/PHP/Go/Scheme…） |
| 运行环境 | 每次判题往工作目录复制运行库（Python 一次 2.4G），靠 `cp`/`mount --bind`/`umount` 拼环境；清理用相对路径 umount 曾误卸别的槽位的 /proc |
| 能力缺口 | 不支持交互题（1644）；守护进程对整份提交 60 秒硬杀（1645 单份约 41 秒，稍慢就被杀） |
| 结果 | Python 抛异常显示成「答案错误」而不是「运行错误」；守护进程崩溃常见（pm2 重启计数上千） |
| 维护 | 32 个语言 .so + 32 份白名单 + 两台机器各自编译；改一处要两台分别回归 |

## 2. 目标与非目标

目标：
1. **对网站零改动**：新判题服务与旧守护进程说同一套 socket.io 协议（`submission` / `judger` / `reject_judge` / `status` / `change` / `error_record`），后端只改 `judger.address`。
2. **安全隔离交给成熟沙箱**：用 [go-judge](https://github.com/criyle/go-judge)（namespace + cgroup + seccomp，国内多家 OJ 在用）执行编译、运行、SPJ、交互器。
3. **判题语义与旧版一致**：全部测试点都跑（all_test_mode）、耗时取单点最大（use_max_time）、Java 系时间 ×2+2 秒/内存 +128MB、ZOJ 文本比较（AC/PE/WA）、diff 报告格式、SPJ 退出码约定、测试运行（problem_id ≤ 0）、查重（sim）。
4. **补能力**：交互题；单份提交不再整体 60 秒硬杀（按测试点限时 + 总时长兜底）；Python 等解释型语言的异常判为运行错误。
5. **可验证**：提供离线回放工具，用新判题机重判历史提交、只比对不写库，逐份对比结论。

非目标（本期不做）：分布式多机调度（继续沿用「先领取再判题」的数据库领取）；改网站显示。

## 3. 架构

```
后端(.16 / .28)  --socket.io v2-->  judge-v2 服务(Node 26, :5110)  --HTTP-->  go-judge(:5050, root, cgroup v1)
       ^                                   |  题目数据 /home/judge/data/<pid>      |  只读挂载宿主 /usr /lib /bin /etc …
       +------------- judger 事件 ----------+  查重 /home/judge/data/<pid>/ac      +  每次运行独立 mount/pid/net/ipc 命名空间
```

- **go-judge**：单进程常驻，只监听 127.0.0.1。沙箱根文件系统按 `mount.yaml` 只读挂载宿主机的编译器与运行库（.28 上现有的 gcc/clang/JDK/Python/Mono/Node…直接复用，不必做镜像），`/w` 为每次运行的可写工作目录，`/tmp` 为 tmpfs。
- **judge-v2**（本仓库 `v2/`，TypeScript）：
  - `server`：socket.io 服务端，兼容旧守护进程协议；内部优先级队列（重判优先级低于新提交，与旧版一致），并发数 = `concurrency`（.28 默认 24）。
  - `pipeline`：编译 → 逐测试点运行 → 比较/SPJ → 汇总，每一步推送 `judger` 进度（state 2 编译 / 3 运行 / 终态）。
  - `languages.json`：每种语言的源文件名、编译命令、运行命令、环境变量、时间/内存加成、进程数上限、需要保留的编译产物。
  - `compare`：ZOJ 文本比较移植 + diff 报告（格式与旧版一致：测试输入/输出/用户输出各前 100 行 + `diff -y` 前 200 行）。
  - `replay`：离线回放 CLI（只读数据库，不写库）。

## 4. 判题流程与结果语义

结果码沿用现有：0 等待、1 重判、2 编译中、3 运行中、4 AC、5 PE、6 WA、7 TLE、8 MLE、9 OLE、10 RE、11 CE、13 测试运行完成、14 排队、16 系统错误。

1. **取数据**：`/home/judge/data/<|pid|>` 下所有 `*.in`（有同名 `.out` 的才算测试点，按文件名排序）。无数据目录 → `reject_judge`（与旧版一致，后端会把该题加入「无数据」名单）。
2. **编译**：源码按语言写成 `Main.<ext>`；编译限制 CPU 10s、实际 20s、内存 2GB、进程 128、输出 1MB；失败 → CE，`compile_info` 为编译器输出（截 64KB）。产物（`Main`、`Main.jar`、`*.class`、`Main.exe`…）放 go-judge 文件缓存，各测试点复用。
3. **运行**：每个测试点一次 go-judge 调用。stdin 直接取测试数据文件；CPU 限时 = 题目时限（Java/Kotlin 系 ×2+2s），实际时间 = 3×CPU+1s；内存限制 = 题目限制（Java 系 +128MB）且 JVM 加 `-Xmx`；输出上限 = max(2×标准输出+1KB, 1MB)；进程数上限按语言。
   - 状态映射：`Time Limit Exceeded`→TLE，`Memory Limit Exceeded`→MLE，`Output Limit Exceeded`→OLE，`Nonzero Exit Status`/`Signalled`→RE（附退出码/信号与 stderr 前 4KB），`Internal Error`→系统错误（不扣学生分，交由重判）。
   - 正常退出 → 比较：有 SPJ 走 SPJ，否则 ZOJ 比较。
4. **SPJ**：`spj`（二进制）/`spj.js`/`spj.py` 三种，调用约定不变：`spj <in> <ans> <user> <code>`，退出码 0=AC，<4 视为 WA，≥4 直接作为结果码；stdout 追加到报告。**SPJ 也在 go-judge 沙箱里跑**（旧版在宿主机以 root 跑），限 CPU 5s、内存 3GB。
5. **交互题**（新增）：数据目录有 `interactor`（testlib 编译产物）时启用。go-judge 用两条管道把选手程序与交互器的 stdin/stdout 互连，交互器参数 `interactor <in> <tout> <ans>`，按 testlib 退出码判定（0 AC、1 WA、2 PE、3 FAIL→系统错误）。
6. **汇总**：全部测试点都跑；最终结果取「最差」（结果码最大，与旧版 `finalACflg` 一致）；任一 PE 且其余 AC → PE；时间取单点最大（TLE 时报时限）；内存取单点最大；`pass_point`=通过数，`pass_rate`=通过数/总数。
7. **测试运行**（problem_id ≤ 0）：用 `custom_input` 跑一次，返回 stdout（截 4KB，附「......Omit N characters.」），超时附中文提示，与旧版相同。
8. **查重**：AC 且语言支持时，沿用旧版 `sim_*` 工具对 `data/<pid>/ac/` 下历史 AC 代码比对，相似度 ≥ 阈值时回传 `sim`/`sim_s_id`；AC 代码存入 `ac/<solution_id>.<ext>`。

## 5. 安全

- 编译/运行/SPJ/交互器全部在 go-judge 沙箱：独立 mount/pid/ipc/uts/net 命名空间（无网络）、只读根文件系统、cgroup 限内存与进程数、seccomp 默认策略。不再需要逐语言维护系统调用白名单。
- go-judge 只监听 127.0.0.1；judge-v2 只监听内网（与旧守护进程一致，由防火墙与绑定地址控制）。
- 不改数据库结构；judge-v2 不直接写库，结果仍由后端落库。

## 6. 上线与回退

1. 在 .28 部署 go-judge 与 judge-v2（新端口 5111），与旧守护进程并存。
2. **离线回放**：抽样历史提交（覆盖全部在用语言、SPJ 题、各种结论），新判题机重判，对比数据库结论；差异逐类分析，直到只剩「旧版判错」或可解释的时间波动。
3. 切换 .28：后端 `judger.address` 改到 5111，重启后端；旧守护进程保留不动，改回地址即回退。
4. .16：后端 `judger.address` 指到 .28 的 judge-v2（统一由 .28 判题，题目数据以 .28 为准）；.16 旧判题机保留可回退。
5. 稳定一周后再下线旧判题机。
