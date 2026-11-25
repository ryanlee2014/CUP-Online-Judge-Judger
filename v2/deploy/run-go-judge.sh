#!/bin/bash
# go-judge launcher for CUP OJ (.28). Run under pm2 as root:
#   pm2 start /opt/kc-gojudge/run-go-judge.sh --name kc-go-judge --interpreter bash
# Swap accounting (memory.memsw) is not enabled on this kernel, so a sandboxed program could
# spill into swap past its memory limit. Set swappiness=0 on go-judge's cgroup subtree so child
# cgroups (created per run) inherit it and stay in RAM.
set -u
D=/opt/kc-gojudge
CG=/sys/fs/cgroup/memory/kcgojudge
"$D/go-judge" -http-addr 127.0.0.1:5050 -mount-conf "$D/mount-cupoj28.yaml" -parallelism "${GOJUDGE_PARALLELISM:-32}" \
  -cgroup-prefix kcgojudge -src-prefix /home/judge/data,$D/share -file-timeout 30m -release &
PID=$!
for i in $(seq 1 50); do
  [ -d "$CG/containers" ] && break
  sleep 0.2
done
for f in "$CG/memory.swappiness" "$CG/containers/memory.swappiness"; do
  [ -e "$f" ] && echo 0 > "$f"
done
wait $PID
