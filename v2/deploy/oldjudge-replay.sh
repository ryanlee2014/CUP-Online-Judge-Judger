#!/bin/bash
# Re-judge solutions with the LEGACY judge_client on this host and current data (for side-by-side with judge v2).
# usage (root): oldjudge-replay.sh <ids-file> <out.tsv>
# Runs each judge in a private net+mount namespace, slot run1 of /home/judge-oldcmp (data -> /home/judge/data).
set -u
IDS=$1; OUT=$2
H=/home/judge-oldcmp
mkdir -p $H/run1 $H/log $H/submission
[ -e $H/data ] || ln -s /home/judge/data $H/data
rsync -a --delete /home/judge/etc/ $H/etc/
chown -R judge:judge $H/run1 $H/log
cd /opt/kc-judge-v2/app
/usr/local/bin/node -e '
const fs=require("fs");const mysql=require("mysql2/promise");
const cfg=JSON.parse(fs.readFileSync("/home/CUP-Online-Judge-Express/config.json","utf8")).mysql;
(async()=>{const c=await mysql.createConnection(cfg);
 const ids=fs.readFileSync(process.argv[1],"utf8").split(/\s+/).filter(Boolean).map(Number);
 fs.mkdirSync("/tmp/oldcmp",{recursive:true});
 for(const id of ids){const [r]=await c.query("select s.solution_id,s.problem_id,s.user_id,s.language,p.time_limit,p.memory_limit,p.spj,sc.source from solution s join problem p using(problem_id) join source_code sc using(solution_id) where s.solution_id=?",[id]);
  if(!r.length)continue;const x=r[0];
  fs.writeFileSync(`/tmp/oldcmp/${id}.json`,JSON.stringify({solution_id:1,source:x.source,custom_input:"",test_run:false,language:x.language,user_id:x.user_id,problem_id:x.problem_id,spj:x.spj==="1",time_limit:Number(x.time_limit),memory_limit:Number(x.memory_limit)}));}
 await c.end();})();' "$IDS"
: > "$OUT"
for f in /tmp/oldcmp/*.json; do
  id=$(basename $f .json)
  unshare -m -n bash -c '
    mount --make-rprivate /
    ip link set lo up
    /usr/local/bin/node -e "const WS=require(\"/home/daemon/node_modules/ws\");const s=new WS.Server({port:5100});s.on(\"connection\",c=>c.on(\"message\",m=>console.log(\"WS: \"+String(m))))" > /tmp/oldcmp/ws.out & W=$!
    sleep 0.5
    timeout 300 /usr/bin/judge_client -solution_id 1 -runner_id 1 -dir '"$H"' -judger_id oldcmp -no-mysql -admin -no-sim -stdin < '"$f"' > /tmp/oldcmp/judge.log 2>&1
    sleep 1; kill $W' 2>/dev/null
  st=$(grep -a "\"finish\":1" /tmp/oldcmp/ws.out | tail -1 | grep -oE "\"state\":[0-9]+" | cut -d: -f2)
  pp=$(grep -a "\"finish\":1" /tmp/oldcmp/ws.out | tail -1 | grep -oE "\"pass_point\":[0-9]+" | cut -d: -f2)
  printf "%s\t%s\t%s\n" "$id" "${st:-NA}" "${pp:-NA}" >> "$OUT"
done
