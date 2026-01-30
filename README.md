# CUP-Online-Judge-Judger
CUP Online Judge 的判题核心，基于 [HUSTOJ](https://github.com/zhblue/hustoj) 改造。

## 使用场景
- 在线评测系统后端判题核心
- 判题集群中的独立判题节点
- 竞赛与作业系统的离线判题服务
- 本地或 CI 环境的提交回归验证

## 功能与特点
- 支持 MySQL 与离线 JSON 两种提交来源
- 支持多语言编译与运行，语言列表由 `language.json` 管理
- 编译命令可配置，按语言 ID 在 `compile.json` 中定义
- 支持普通判题与 Special Judge
- 支持相似度检测（sim）并可按配置开关
- 支持多测试点串行或并行执行
- 支持自定义输入的测试运行模式
- 支持时间与内存限制控制
- 支持 chroot + seccomp/ptrace 沙箱隔离
- 支持共享内存运行目录以提升性能
- 支持详细结果输出与运行信息回传

## 判题能力与实现机制

### 判题流程
- 编译：使用 `compile.json` 配置的编译命令，失败则直接返回 CE
- 运行：子进程执行用户程序，父进程监控资源与退出状态
- 判题：普通题使用 Compare 模块比对输出；SPJ 走 Special Judge
- 入口实现参考 [judge_solution](judge_client.cpp#L560-L603)、[configure_and_run](judge_client.cpp#L189-L238)

### 结果判定与多测试点
- 单测点与多测点都经过统一的 `judge_solution` 判定逻辑
- `all_test_mode` 为 1 时强制跑完所有测试点并按通过率汇总
- `use_max_time` 控制用时统计为最大用时或累计用时
- 并行判题通过线程池并发执行，入口为 [runParallelJudge](judge_client.cpp#L643-L663)

### 沙箱与资源限制
- 运行前构建 chroot 环境，复制必要的系统库与 busybox，见 [buildRuntime](model/judge/language/Language.cpp#L70-L82)
- 运行时设置 CPU、文件大小、栈与内存限制，见 [configure_and_run](judge_client.cpp#L189-L238)
- `use_ptrace=1` 使用 ptrace 监控；否则使用 seccomp 过滤系统调用
- 进程数限制默认 1，见 [setProcessLimit](model/judge/language/Language.cpp#L13-L17)

### Special Judge 设计
- SPJ 文件位于 `data/{problem_id}/`，按 `spj` / `spj.js` / `spj.py` 优先级加载
- 使用 `fork + pipe` 将 SPJ 结果从子进程回传，避免影响主判题进程
- SPJ 侧默认限制 CPU 5s、输出文件 50MB，并限制进程数
- 实现见 [SpecialJudge::run](model/judge/policy/SpecialJudge.cpp#L46-L120)

### Syscall 限制与记录
- 每种语言对应一套 syscall 白名单，定义在 `model/judge/language/syscall/*/syscall32.h|syscall64.h`
- 初始化阶段调用 [initSyscallLimits](manager/syscall/InitManager.cpp#L10-L23) 将白名单写入计数器
- 未启用 ptrace 时使用 seccomp 过滤，构建逻辑见 [seccomp_helper](model/judge/language/common/seccomp_helper.cpp#L98-L117)
- 通过 `-record` 可记录 syscall 使用情况，并输出为可复用的白名单数组

## 组件与入口
- judge_client：主判题进程，负责编译、运行与判题
- wsjudged：判题进程包装器，设置资源限制后启动 judge_client
- shell/djudge.sh：调试用脚本示例

## 部署

### 第一次部署
- `git clone`
- 执行仓库根目录下的 `./build.sh`
- 配置 `/home/judge/etc/config.json` 与 `/home/judge/etc/compile.json`
- 将 `wsjudged` 移动到 `CUP-Online-Judge-Judger-Daemon-Service` 下
- 启动 `CUP-Online-Judge-Judger-Daemon-Service`

### 目录约定
- `/home/judge`：默认判题根目录（可通过参数覆盖）
- `/home/judge/etc`：配置目录
- `/home/judge/submission`：无 MySQL 模式下的提交描述文件
- `/home/judge/data/{problem_id}`：测试数据目录，存放 `.in/.out` 与 `spj` 文件
- `/dev/shm/cupoj`：共享内存运行目录（按配置启用）

## 使用说明

### 方式一：MySQL 模式（默认）
judge_client 直接从数据库读取题目与提交信息。

```bash
judge_client \
  -solution_id 1001 \
  -runner_id 1 \
  -dir /home/judge \
  -judger_id j1
```

### 方式二：无 MySQL / JSON 模式
通过 `-no-mysql` 指定为离线模式，提交信息从 JSON 文件或标准输入读取。

```bash
judge_client \
  -solution_id 1001 \
  -runner_id 1 \
  -dir /home/judge \
  -judger_id j1 \
  -no-mysql
```

默认读取 `/home/judge/submission/{judger_id}.json`，也可使用 `-stdin` 从标准输入读 JSON。

```bash
echo '{"language":1,"user_id":"u1","problem_id":1001,"spj":false,"memory_limit":256,"time_limit":1.5,"source":"int main(){}","solution_id":1001}' \
  | judge_client -solution_id 1001 -runner_id 1 -dir /home/judge -judger_id j1 -no-mysql -stdin
```

### wsjudged 调用
```bash
wsjudged <solution_id> <runner_id> <oj_home_dir> [DEBUG]
```

## 入参说明
参数由 [detectArgType](library/judge_lib.h.cpp#L239-L274) 识别，[init_parameters](judge_client.cpp#L665-L748) 解析。

- `-solution_id <id>`：提交 ID
- `-runner_id <id>`：判题进程 ID，用于区分运行目录
- `-dir <path>`：判题根目录，默认 `/home/judge`
- `-judger_id <id>`：用于定位 `/submission/{id}.json` 的 ID
- `-language <name>`：设置语言名称（主要用于 syscall 统计输出）
- `-no-mysql`：不读数据库，走 JSON 提交模式
- `-stdin`：从标准输入读取提交 JSON（配合 `-no-mysql`）
- `-no_record`：关闭 syscall 记录
- `-record`：开启 syscall 记录
- `-admin`：管理员模式，跳过相似度检测
- `-no-sim`：禁用相似度检测
- `DEBUG`：开启调试日志

兼容旧参数格式：
```
judge_client <solution_id> <runner_id> [oj_home_dir] [lang_or_debug] [DEBUG]
```

## 配置项

### /home/judge/etc/config.json
judge_client 实际读取的配置文件，加载逻辑见 [init_mysql_conf](judge_client.cpp#L95-L132)。

- `hostname` / `username` / `password` / `db_name` / `port`：数据库连接
- `java_time_bonus` / `java_memory_bonus`：Java 语言额外时空限
- `java_xms` / `java_xmx`：Java 编译运行 JVM 参数
- `sim_enable`：是否启用相似度检测
- `full_diff`：输出差异模式
- `judger_name`：上报使用的判题机标识
- `shm_run`：是否启用共享内存运行目录
- `use_max_time`：是否使用最大用时统计
- `use_ptrace`：是否启用 ptrace 监控
- `all_test_mode`：是否强制跑完所有测试点
- `enable_parallel`：是否启用并行判题

### /home/judge/etc/compile.json
编译命令配置，judge_client 在编译阶段读取该文件，键为语言 ID，值为编译参数数组。

### /home/judge/etc/language.json
语言 ID 与语言名称映射，供运行时与编译阶段使用。

### /home/judge/etc/judge.conf
部署脚本会写入该文件，便于与其他服务保持一致；judge_client 主要读取 config.json。

## 提交 JSON 格式（无 MySQL 模式）
字段定义见 [SubmissionInfo](model/submission/SubmissionInfo.cpp#L102-L113)。

```json
{
  "language": 1,
  "user_id": "u1",
  "problem_id": 1001,
  "spj": false,
  "memory_limit": 256,
  "time_limit": 1.5,
  "source": "int main(){}",
  "solution_id": 1001
}
```

若需要自定义输入模式，可额外提供：
```json
{
  "test_run": true,
  "custom_input": "1 2\n"
}
```
