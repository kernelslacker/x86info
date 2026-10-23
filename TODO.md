# x86info TODO

## 🔴 Correctness

## 🟡 Performance

## 🟡 Maintainability

## 🟡 Clean-ups/Refactoring/Style

## 🟡 Deprecated/Obsolete API Usage

## 🟡 Better API Alternatives

## 🟡 Architectural improvements

## 🟡 Comments

## 🟡 Tests & docs

I1. `make check` replays CPUID dumps from tests/dumps and diffs the output
    with the .out goldens, but only a Zen 5 dump is checked in. Add dumps
    (`x86info -r | grep '^eax in'`) from a K7, K8, Zen 2/3/4, Core 2,
    Nehalem, Alder Lake hybrid and VIA CPU, then `UPDATE=1
    tests/run-tests.sh tests/build/x86info` after checking the output by
    hand. [needs hardware]

## 🟢 Enhancements

## 🟢 Minor nits / one-liners

