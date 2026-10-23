#!/bin/sh
# Check that run-tests.sh fails when the tested binary exits unsuccessfully
# after printing the golden output, and that UPDATE=1 leaves the goldens alone.

dir=$(dirname "$0")
tmp=$(mktemp -d) || exit 1
trap 'rm -rf "$tmp"' EXIT

mkdir "$tmp/dumps"
cp "$dir"/dumps/zen5-smt-pair.dump "$dir"/dumps/zen5-smt-pair.out "$tmp/dumps/"
cp "$dir/run-tests.sh" "$tmp/"

cat > "$tmp/fake" <<'FAKE'
#!/bin/sh
echo "x86info vX"
cat "${X86INFO_CPUID_DUMP%.dump}.out"
exit 1
FAKE
chmod +x "$tmp/fake"

if "$tmp/run-tests.sh" "$tmp/fake" > /dev/null; then
	echo "run-tests.sh passed a binary that exited 1"
	exit 1
fi

cp "$tmp/dumps/zen5-smt-pair.out" "$tmp/golden"
UPDATE=1 "$tmp/run-tests.sh" "$tmp/fake" > /dev/null
if ! cmp -s "$tmp/golden" "$tmp/dumps/zen5-smt-pair.out"; then
	echo "UPDATE=1 rewrote a golden from a failed run"
	exit 1
fi
