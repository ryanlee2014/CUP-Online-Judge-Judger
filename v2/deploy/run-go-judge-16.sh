#!/bin/bash
# go-judge launcher for CUP OJ acm (.16, CentOS 7.9, kernel 3.10). Run under pm2 as root:
#   PM2_HOME=/root/.pm2-n26 /opt/node26/bin/pm2 start /opt/kc-gojudge/run-go-judge-16.sh --name kc-go-judge --interpreter bash
# .16 has user.max_user_namespaces=0, so go-judge is a build patched with
# go-judge-patch/go-sandbox-v0.14.6-no-userns.patch and started with KC_NO_USERNS=1:
# the container init stays host root inside fresh mount/pid/net/ipc/uts namespaces, and
# user programs run as uid/gid 61536 (no host account) with no capabilities, no
# supplementary groups and seccomp.
set -u
D=/opt/kc-gojudge
CG=/sys/fs/cgroup/memory/kcgojudge
KC_NO_USERNS=1 "$D/go-judge-kc" -http-addr 127.0.0.1:5050 -mount-conf "$D/mount-cupoj16-nouserns.yaml" \
  -parallelism "${GOJUDGE_PARALLELISM:-8}" -cgroup-prefix kcgojudge -container-cred-start 61536 \
  -src-prefix /home/judge/data,$D/share -file-timeout 30m -release &
PID=$!
for i in $(seq 1 50); do
  [ -d "$CG/containers" ] && break
  sleep 0.2
done
for f in "$CG/memory.swappiness" "$CG/containers/memory.swappiness"; do
  [ -e "$f" ] && echo 0 > "$f"
done
wait $PID
