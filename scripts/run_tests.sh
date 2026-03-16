#!/usr/bin/env bash
set -u

BASIC_BIN="${BASIC_BIN:-bin/linux/basic}"
CASES_DIR="tests/cases"
EXPECTED_DIR="tests/expected"

if [ ! -x "$BASIC_BIN" ]; then
  echo "error: BASIC binary not found or not executable: $BASIC_BIN" >&2
  echo "hint: run 'make linux-nobeep' or set BASIC_BIN=..." >&2
  exit 2
fi

fail=0
pass=0

for case_file in "$CASES_DIR"/*.BAS; do
  base=$(basename "$case_file" .BAS)
  expected="$EXPECTED_DIR/$base.out"
  if [ ! -f "$expected" ]; then
    echo "skip: $base (missing expected file)"
    continue
  fi

  tmp_out=$(mktemp)
  "$BASIC_BIN" "$CASES_DIR/$base" >"$tmp_out" 2>&1

  if diff -u "$expected" "$tmp_out" >/dev/null; then
    echo "pass: $base"
    pass=$((pass + 1))
  else
    echo "fail: $base"
    diff -u "$expected" "$tmp_out" || true
    fail=$((fail + 1))
  fi
  rm -f "$tmp_out"

done

rm -f _testio _testin

echo ""
echo "passed: $pass"
echo "failed: $fail"

if [ "$fail" -ne 0 ]; then
  exit 1
fi
