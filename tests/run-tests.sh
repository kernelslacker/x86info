#!/bin/sh
# Run the replay build of x86info against each dump and diff the output with
# the matching .out (default options) and .v.out (-v) files, and check that
# the raw CPUID dump it prints replays to the same dump. Regenerate the
# goldens with UPDATE=1.
#
# usage: run-tests.sh <x86info-replay-binary>

bin=$1
dir=$(dirname "$0")/dumps
fail=0
tmp=$(mktemp -d) || exit 1
trap 'rm -rf "$tmp"' EXIT

# golden <dump> <suffix> <options...>: compare the output against <dump>.<suffix>
golden() {
	dump=$1
	suffix=$2
	shift 2
	name=$(basename "$dump")-$suffix
	golden=${dump%.dump}.$suffix

	# shellcheck disable=SC2068
	if ! raw=$(X86INFO_CPUID_DUMP=$dump "$bin" $@); then
		echo "FAIL $name: $bin exited unsuccessfully"
		fail=1
		return
	fi
	actual=$(printf '%s\n' "$raw" | tail -n +2)
	if [ -n "$UPDATE" ]; then
		printf '%s\n' "$actual" > "$golden"
	elif printf '%s\n' "$actual" | diff -u "$golden" - > /dev/null; then
		echo "PASS $name"
	else
		echo "FAIL $name"
		printf '%s\n' "$actual" | diff -u "$golden" -
		fail=1
	fi
}

# roundtrip <dump>: the raw dump of a replay must itself replay unchanged
roundtrip() {
	dump=$1
	name=$(basename "$dump")-roundtrip
	first=$tmp/first.dump
	second=$tmp/second.dump

	if ! X86INFO_CPUID_DUMP=$dump "$bin" -r --all-cpus |
	     grep -E '^(CPU #[0-9]+:|eax in:)' > "$first" ||
	   ! X86INFO_CPUID_DUMP=$first "$bin" -r --all-cpus |
	     grep -E '^(CPU #[0-9]+:|eax in:)' > "$second"; then
		echo "FAIL $name: raw dump failed"
		fail=1
	elif diff -u "$first" "$second" > /dev/null; then
		echo "PASS $name"
	else
		echo "FAIL $name"
		diff -u "$first" "$second"
		fail=1
	fi
}

for dump in "$dir"/*.dump; do
	golden "$dump" out -c -f --addr --bugs --connector
	golden "$dump" v.out -v
	roundtrip "$dump"
done

exit $fail
