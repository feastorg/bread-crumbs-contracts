# Compile Smoke

This folder contains minimal compile-only checks for the public contract headers.

## Purpose

Catch accidental header breakage (missing includes, symbol regressions) without requiring hardware.

## Notes

- `smoke.c` includes `<bread/bread_ops.h>`.
- Build requires CRUMBS headers on include path.
- This is intended for CI or local toolchain validation.
