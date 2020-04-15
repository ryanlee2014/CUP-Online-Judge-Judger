#!/bin/bash
CPU_CORE=$(nproc)
./shell/build_dependency.sh
cmake ./ && make -j$CPU_CORE && chmod a+x judge_client && mv ./judge_client /usr/bin
useradd -m -u 1536 judge
mkdir /home/judge
mkdir /home/judge/etc
mkdir /dev/shm/cupoj
mkdir /dev/shm/cupoj/submission
mkdir /home/judge/submission
chown judge /home/judge/*
chgrp judge /home/judge/*
cp -r etc/* /home/judge/etc/ &&
chown -R judge /home/judge &&
chgrp -R judge /home/judge &&
./shell/build_source_code.sh
chgrp -R judge /usr/lib/cupjudge &&
chown -R judge /usr/lib/cupjudge