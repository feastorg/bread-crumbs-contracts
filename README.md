# bread-crumbs-contracts

Header-only CRUMBS operation contracts for the BREAD Slice family.

This repository is the single source of truth for BREAD wire contracts used by:

- Slice peripheral firmware repos (device side)
- Controller repos (Linux/Arduino/etc., controller side)

## Public Contract Surface

Canonical public headers are generation-free and device-capability based:

```c
#include <bread/bread_ops.h>
#include <bread/dcmt_ops.h>
#include <bread/rlht_ops.h>
#include <bread/bread_caps.h>
```

## Capability Discovery

BREAD uses a shared capability query opcode:

- `BREAD_OP_GET_CAPS`

Payload v1:

- `[caps_schema:u8][caps_level:u8][caps_flags:u32_le]`

Controllers should use: version query -> caps query -> command gating.

## Current Device Contracts

- RLHT: `include/bread/rlht_ops.h`
- DCMT: `include/bread/dcmt_ops.h`
- Shared caps helpers: `include/bread/bread_caps.h`
- Shared version helpers: `include/bread/bread_version_helpers.h`

## Quick Start (PlatformIO)

In your project `platformio.ini`:

```ini
lib_deps =
  cameronbrooks11/CRUMBS @ ^0.11.0
  cameronbrooks11/bread-crumbs-contracts @ ^0.2.0
```

Controller source:

```c
#include <bread/bread_ops.h>
```

## Linux Example Controllers

This repo includes interactive Linux controller examples for slice testing:

- `examples/controller_discovery` (auto-discovery + compatibility/caps probing)
- `examples/controller_manual` (fixed address map from config + capability gating)

Build example:

```bash
cd examples/controller_discovery
mkdir -p build && cd build
cmake ..
make
./controller_discovery /dev/i2c-1
```

Default CMake assumes CRUMBS repo exists at sibling path `../CRUMBS` from this repo root.
Override with `-DCRUMBS_PATH=/path/to/CRUMBS` if needed.

## Quick Start (CMake)

Add this repo as a source dependency, then include canonical headers:

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
- `docs/compatibility-matrix.md`
- `docs/capability-oriented-contract-policy.md`
- `docs/capability-implementation-plan.md`

## Layout

```text
include/bread/
  bread_ops.h
  bread_caps.h
  bread_version_helpers.h
  dcmt_ops.h
  rlht_ops.h

docs/
  versioning.md
  type-id-registry.md
  compatibility.md
  compatibility-matrix.md
  capability-oriented-contract-policy.md
  capability-implementation-plan.md
  generation-contract-strategy.md

examples/
  controller_discovery/
  controller_manual/

tests/compile_smoke/
  smoke.c
```
