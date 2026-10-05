# Capabilities

## Shared Capability Query

- Opcode: `BREAD_OP_GET_CAPS` (`0x7F`)
- Requested using CRUMBS reply-query path (`CRUMBS_CMD_SET_REPLY`)

## Payload Schema v1

Payload bytes:

- `caps_schema` (`u8`)
- `caps_level` (`u8`)
- `caps_flags` (`u32`, little-endian)

Schema constants:

- `BREAD_CAPS_SCHEMA_V1 = 0x01`
- `BREAD_CAPS_V1_PAYLOAD_LEN = 6`

## Parsing Rules

Controllers should:

1. validate minimum payload length,
2. validate/record schema,
3. treat `caps_flags` as authoritative for command gating,
4. treat `caps_level` as a coarse profile hint.

Unknown flags must be ignored.

## Legacy Fallback

If capability query is unavailable (timeout/unsupported), assume baseline capabilities for that type.
Do not assume optional features.

## DCMT Capability Registry

Levels:

- `DCMT_CAP_LEVEL_1`: open-loop + brake baseline
- `DCMT_CAP_LEVEL_2`: Level 1 + closed-loop position + PID tuning
- `DCMT_CAP_LEVEL_3`: Level 2 + closed-loop speed

Flags:

- bit 0: `DCMT_CAP_OPEN_LOOP_CONTROL`
- bit 1: `DCMT_CAP_BRAKE_CONTROL`
- bit 2: `DCMT_CAP_CLOSED_LOOP_POSITION`
- bit 3: `DCMT_CAP_CLOSED_LOOP_SPEED`
- bit 4: `DCMT_CAP_PID_TUNING`
- bit 5: `DCMT_CAP_CMD_WATCHDOG` — firmware supports the shared bus-liveness
  command watchdog (`BREAD_OP_SET_WATCHDOG` / `BREAD_OP_GET_WATCHDOG`);
  controllers must not send `BREAD_OP_SET_WATCHDOG` unless this flag is
  advertised
- bit 6: `DCMT_CAP_CLEAR_WATCHDOG_TRIP` — the watchdog trip latches until
  `BREAD_OP_CLEAR_WATCHDOG_TRIP`, a deliberate local operator command on the
  slice (firmware-defined, not any serial input), or a reboot;
  `BREAD_OP_SET_WATCHDOG` and ordinary frames no longer clear it.
  Advertised only together with `DCMT_CAP_CMD_WATCHDOG`. Controllers must
  not send `BREAD_OP_CLEAR_WATCHDOG_TRIP` unless this flag is advertised; on
  firmware without it, `BREAD_OP_SET_WATCHDOG` clears the trip

Baseline flag set:

- `DCMT_CAP_BASELINE_FLAGS = OPEN_LOOP_CONTROL | BRAKE_CONTROL`

## RLHT Capability Registry

Levels:

- `RLHT_CAP_LEVEL_1`: baseline RLHT profile
- `RLHT_CAP_LEVEL_2`: reserved
- `RLHT_CAP_LEVEL_3`: reserved

Flags:

- bit 0: `RLHT_CAP_MODE_CONTROL`
- bit 1: `RLHT_CAP_SETPOINT_CONTROL`
- bit 2: `RLHT_CAP_PID_TUNING`
- bit 3: `RLHT_CAP_PERIOD_CONTROL`
- bit 4: `RLHT_CAP_TC_SELECT`
- bit 5: `RLHT_CAP_OPEN_DUTY_CONTROL`
- bit 6: `RLHT_CAP_CMD_WATCHDOG` — firmware supports the shared bus-liveness
  command watchdog (`BREAD_OP_SET_WATCHDOG` / `BREAD_OP_GET_WATCHDOG`);
  controllers must not send `BREAD_OP_SET_WATCHDOG` unless this flag is
  advertised
- bit 7: `RLHT_CAP_CLEAR_WATCHDOG_TRIP` — the watchdog trip latches until
  `BREAD_OP_CLEAR_WATCHDOG_TRIP`, a deliberate local operator command on the
  slice (firmware-defined, not any serial input), or a reboot;
  `BREAD_OP_SET_WATCHDOG` and ordinary frames no longer clear it.
  Advertised only together with `RLHT_CAP_CMD_WATCHDOG`. Controllers must
  not send `BREAD_OP_CLEAR_WATCHDOG_TRIP` unless this flag is advertised; on
  firmware without it, `BREAD_OP_SET_WATCHDOG` clears the trip

Baseline flag set:

- `RLHT_CAP_BASELINE_FLAGS` (all currently supported RLHT control features;
  does not include `RLHT_CAP_CMD_WATCHDOG` or `RLHT_CAP_CLEAR_WATCHDOG_TRIP`)

## Extension Policy

Capability evolution is additive-only:

- add new flags for optional new behavior,
- preserve semantics of existing flags/opcodes,
- append payload fields only through a new schema version.

One exception: firmware that advertises `DCMT_CAP_CLEAR_WATCHDOG_TRIP` /
`RLHT_CAP_CLEAR_WATCHDOG_TRIP` changes what `BREAD_OP_SET_WATCHDOG` does,
because it no longer clears a watchdog trip. A controller that predates the
flag has no remote way to clear a trip on that firmware, so the controller
release that understands the flag must ship with or before such firmware
(see `controller-compatibility.md`).
