# bread-crumbs-contracts

Header-only CRUMBS operation contracts for the BREAD Slice family.

This repository is the single source of truth for BREAD wire contracts used by:

- Slice peripheral firmware repos (device side)
- controller repos (Linux/Arduino/etc., controller side)

## Current Device Contracts

- RLHT (`RLHT_TYPE_ID = 0x01`): `include/bread/rlht_ops.h`
- DCMT (`DCMT_TYPE_ID = 0x02`): `include/bread/dcmt_ops.h`

Use `include/bread/bread_ops.h` for one-stop family includes plus shared version parsing/compat helpers.

## Quick Start (PlatformIO)

In your project `platformio.ini`:

```ini
lib_deps =
  cameronbrooks11/CRUMBS @ ^0.11.0
  https://github.com/FEASTorg/bread-crumbs-contracts.git#v0.1.0
```

Controller source:

```c
#include <bread/bread_ops.h>
```

## Quick Start (CMake)

Add this repo as a source dependency, then include:

```c
#include <bread/bread_ops.h>
```

Ensure CRUMBS headers are also on include path, since contract headers depend on:

- `crumbs.h`
- `crumbs_message_helpers.h`
- `crumbs_version.h`

## Versioning Model

Contract versioning follows semver and wire compatibility:

- MAJOR: breaking wire change
- MINOR: backward-compatible wire additions
- PATCH: non-wire changes (docs/tooling/comments)

See:

- `docs/versioning.md`
- `docs/type-id-registry.md`
- `docs/compatibility.md`

## Layout

```text
include/bread/
  bread_ops.h
  dcmt_ops.h
  rlht_ops.h
docs/
  versioning.md
  type-id-registry.md
  compatibility.md
tests/compile_smoke/
  smoke.c
```

## Status

Initial bootstrap baseline extracted from stabilized Slice_DCMT and Slice_RLHT contracts.
