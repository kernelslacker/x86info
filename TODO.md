# TODO

Whole-tree review. `make check` passes. Items marked "unverified" come
from reading the code only; confirm them before fixing.

## 🔴 Correctness

## 🟡 Maintainability

B8. `vendors/intel/identify-family6-extended.c:209`,
    `identify-family6.c:299,309` — identification calls `ensure_MHz()`,
    so naming a Core 2 CPU can spin for 100 ms per CPU. Move the
    frequency-keyed SKU refinement to display time, or pass the rate in.
    See C2. (sonnet)

## 🟡 Tests & docs

## 🟢 Enhancements

## 🟢 Minor nits

