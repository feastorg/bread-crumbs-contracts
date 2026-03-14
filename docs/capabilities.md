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

- `DCMT_CAP_LEVEL_1`: baseline open-loop profile
- `DCMT_CAP_LEVEL_2`: closed-loop capable profile
- `DCMT_CAP_LEVEL_3`: reserved

Flags:

- bit 0: `DCMT_CAP_OPEN_LOOP_CONTROL`
- bit 1: `DCMT_CAP_BRAKE_CONTROL`
- bit 2: `DCMT_CAP_CLOSED_LOOP_POSITION`
- bit 3: `DCMT_CAP_CLOSED_LOOP_SPEED`
- bit 4: `DCMT_CAP_PID_TUNING`

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

Baseline flag set:

- `RLHT_CAP_BASELINE_FLAGS` (all currently supported RLHT control features)

## Extension Policy

Capability evolution is additive-only:

- add new flags for optional new behavior,
- preserve semantics of existing flags/opcodes,
- append payload fields only through a new schema version.
