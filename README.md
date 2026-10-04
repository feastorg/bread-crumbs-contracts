# bread-crumbs-contracts

Header-only CRUMBS operation contracts for BREADS-compatible Slices.

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

Controller flow: version query -> caps query -> command gating.

## Command Watchdog

BREAD defines a shared bus-liveness command watchdog, gated per type by a
capability flag (`DCMT_CAP_CMD_WATCHDOG`, `RLHT_CAP_CMD_WATCHDOG`):

- `BREAD_OP_SET_WATCHDOG`: `[timeout_ms:u16]`, 0 = disarm (firmware boots disarmed)
- `BREAD_OP_GET_WATCHDOG`: `[armed:u8][timeout_ms:u16][tripped:u8][trip_count:u8]`

While armed, any valid inbound frame refreshes it; expiry drives the slice's
actuators to their safe state without touching estop.

## Current Device Contracts

- RLHT: `include/bread/rlht_ops.h`
- DCMT: `include/bread/dcmt_ops.h`
- Shared caps helpers: `include/bread/bread_caps.h`
- Shared watchdog helpers: `include/bread/bread_watchdog.h`
- Shared version helpers: `include/bread/bread_version_helpers.h`

## Quick Start (PlatformIO)

In your project `platformio.ini`:

```ini
lib_deps =
  cameronbrooks11/CRUMBS @ ^0.12.0
  cameronbrooks11/bread-crumbs-contracts @ ^0.4.0
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

## Python Codec

`bindings/python` is a pure-Python codec for the same contracts, for
controllers written in Python. It mirrors the headers one module per header
and names every function after its C helper: `rlht_send_set_setpoints()`
returns the payload bytes `rlht_send_set_setpoints()` puts on the wire,
`dcmt_parse_state_payload()` returns a frozen `DcmtStateResult`, and the
`*_query_*` functions return the one-byte SET_REPLY payload. Framing, CRC
and I2C stay in the transport (CRUMBS, or a Python binding of it); the
codec stops at type id, opcode and payload.

Install from source until a release is on PyPI (#21); Python 3.11 or newer,
no runtime dependencies:

```bash
pip install git+https://github.com/feastorg/bread-crumbs-contracts.git
```

```python
from bread_crumbs_contracts import (
    CRUMBS_CMD_SET_REPLY,
    CRUMBS_TYPE_ID_ANY,
    RLHT_OP_SET_SETPOINTS,
    RLHT_TYPE_ID,
    rlht_parse_state_payload,
    rlht_query_state,
    rlht_send_set_setpoints,
)

# SET_SETPOINTS at 25.0 C and -1.0 C. Send the payload with RLHT_TYPE_ID and
# RLHT_OP_SET_SETPOINTS through the CRUMBS transport; arguments outside the
# field's range raise ValueError before anything is encoded.
payload = rlht_send_set_setpoints(sp1_deci_c=250, sp2_deci_c=-10)
assert (RLHT_TYPE_ID, RLHT_OP_SET_SETPOINTS, payload.hex()) == (0x01, 0x02, "fa00f6ff")

# A query is a SET_REPLY frame whose payload names the reply to build next.
request = rlht_query_state()
assert (CRUMBS_TYPE_ID_ANY, CRUMBS_CMD_SET_REPLY, request.hex()) == (0x00, 0xFE, "80")

# The GET_STATE reply payload read back through the transport. Each parser
# applies the length rule of its C namesake: rlht_parse_state_payload reads
# the 19-byte prefix and ignores trailing bytes; dcmt_parse_state_payload
# and bread_watchdog_parse_payload require the exact length (#18 is where
# the rule becomes uniform).
state = rlht_parse_state_payload(bytes.fromhex("0002f401f6fffa002c016400c800e803d00706"))
assert (state.t1_deci_c, state.t2_deci_c, state.tc1, state.tc2) == (500, -10, 2, 1)
```

The codec cannot drift from the headers: `tests/golden_vectors/gen_vectors.c`
calls every C send and parse helper and writes `tests/golden_vectors/vectors.json`,
the Python tests check every constant, encoder and parser against it, and CI
regenerates the file and fails on any difference. To regenerate after a
header change, build the test targets and run the generator (see
`tests/golden_vectors/README.md`):

```bash
cmake --build build/contracts --target bread_contracts_gen_vectors
./build/contracts/bread_contracts_gen_vectors tests/golden_vectors/vectors.json
```

Development uses [uv](https://docs.astral.sh/uv/): `uv sync` installs the
locked tools, then `uv run ruff check`, `uv run pyright` and `uv run pytest`
are what CI runs.

## Documentation

- `docs/overview.md`
- `docs/contract-model.md`
- `docs/capabilities.md`
- `docs/controller-compatibility.md`
- `docs/type-ids.md`
- `docs/protocol-versioning.md`

## Layout

```text
include/bread/
  bread_ops.h
  bread_caps.h
  bread_version_helpers.h
  dcmt_ops.h
  rlht_ops.h

docs/
  overview.md
  contract-model.md
  capabilities.md
  controller-compatibility.md
  type-ids.md
  protocol-versioning.md

examples/
  controller_discovery/
  controller_manual/

bindings/python/
  src/bread_crumbs_contracts/
  tests/

tests/compile_smoke/
  smoke.c

tests/golden_vectors/
  gen_vectors.c
  vectors.json
```
