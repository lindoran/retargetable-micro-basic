# BASIC Test Suite

This folder contains runnable `.BAS` tests and expected outputs.

## Layout

- `tests/cases/*.BAS` — test programs
- `tests/expected/*.out` — expected stdout
- `scripts/run_tests.sh` — runner script

## Run

```
make test
```

Or directly:

```
BASIC_BIN=bin/linux/basic ./scripts/run_tests.sh
```

Notes:

- Tests run the interpreter in file mode, so the banner is suppressed.
- Expected outputs are exact; failures show a unified diff.
- Some tests create temporary files (`_testio`, `_testin`) which the runner removes.
