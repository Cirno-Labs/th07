#!/bin/sh
# Decompiles functions from the Ghidra project made by analyze.sh into
# build/decomp/<address>.c. With no arguments, every function.
# GHIDRA defaults to ~/opt/ghidra_*.
set -e
cd "$(dirname "$0")/../.."
GHIDRA=${GHIDRA:-$(ls -d ~/opt/ghidra_* | tail -1)}
"$GHIDRA/support/analyzeHeadless" build/ghidra th07_100b \
	-process th07_100b.exe -noanalysis -readOnly \
	-scriptPath tools/ghidra \
	-postScript Decompile.java "$PWD/build/decomp" "$@" \
	> build/ghidra/decompile.log 2>&1
grep REPORT build/ghidra/decompile.log
