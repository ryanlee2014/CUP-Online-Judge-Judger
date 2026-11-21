#!/bin/sh
# Go compile wrapper for the judge sandbox: the compile step runs as the judge user inside a chroot
# where $HOME (/root) is not writable, so point Go caches at the sandbox /tmp.
# Install: cp shell/go-judge.sh /usr/local/bin/go-judge && chmod 755 /usr/local/bin/go-judge
export HOME=/tmp GOCACHE=/tmp/gocache GOPATH=/tmp/gopath GOTMPDIR=/tmp
exec /usr/bin/go "$@"
