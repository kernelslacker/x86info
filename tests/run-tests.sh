#!/bin/sh
# Run the replay build of x86info against each dump and diff the output with
# the matching .out file. Regenerate the goldens with UPDATE=1.
#
# usage: run-tests.sh <x86info-replay-binary>

bin=$1
dir=$(dirname "$0")/dumps
fail=0

for dump in "$dir"/*.dump; do
	golden=${dump%.dump}.out
	if ! raw=$(X86INFO_CPUID_DUMP=$dump "$bin" -c -f --addr --bugs --connector); then
		echo "FAIL $(basename "$dump"): $bin exited unsuccessfully"
		fail=1
		continue
	fi
	actual=$(printf '%s\n' "$raw" | tail -n +2)
	if [ -n "$UPDATE" ]; then
		printf '%s\n' "$actual" > "$golden"
	elif printf '%s\n' "$actual" | diff -u "$golden" - > /dev/null; then
		echo "PASS $(basename "$dump")"
	else
		echo "FAIL $(basename "$dump")"
		printf '%s\n' "$actual" | diff -u "$golden" -
		fail=1
	fi
done

exit $fail
