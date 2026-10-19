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

- bit 0: `DCMT_CAP_OPEN_LOOP_CONTROL` — `SET_OPEN_LOOP` takes a signed PWM
  per motor, `-DCMT_PWM_MAX` to `DCMT_PWM_MAX` (255) inclusive. Firmware
  advertising `DCMT_CAP_REFUSES` leaves a PWM outside that unchanged and
  refuses with `BREAD_REFUSE_UNDEFINED_VALUE`; older firmware clamps it
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
- bit 7: `DCMT_CAP_ESTOP_HOLD` — a hardware e-stop holds the way a latched
  watchdog trip does. On a press the firmware sets both brake flags and
  moves the position setpoints to the encoders, re-syncing them while the
  e-stop is held. While e-stopped it refuses `SET_OPEN_LOOP`,
  `SET_SETPOINT`, `SET_MODE` and any brake release. Releasing the e-stop
  moves nothing; the controller resumes with `SET_BRAKE(0,0)`, then
  setpoints, then the mode. On firmware without the flag, a release
  resumes the stored mode and setpoints, including commands sent during
  the hold
- bit 8: `DCMT_CAP_REFUSES` — a SET the firmware does not act on in full
  is refused with `crumbs_refuse()` and a `BREAD_REFUSE_*` reason
  (`bread_refuse.h`), so GET_ACK's applied means every part was acted on as
  sent. A frame acted on in part applies what it can and is refused with
  the reason of its first ignored part in payload order; a SET_BRAKE while
  tripped engages any brake it names and is refused with
  `BREAD_REFUSE_WATCHDOG_TRIP` if it asked for a release. After a refusal
  the controller reads GET_STATE to see what took effect. On firmware
  without the flag every frame reports applied, including ones the
  firmware ignored

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
- bit 3: `RLHT_CAP_PERIOD_CONTROL` — `SET_PERIODS` takes a relay period
  per heater, `RLHT_PERIOD_MIN_MS` (100) to `RLHT_PERIOD_MAX_MS` (10000) ms
  inclusive. Firmware advertising `RLHT_CAP_REFUSES` leaves a period outside
  that unchanged and refuses with `BREAD_REFUSE_UNDEFINED_VALUE`; older
  firmware clamps it
- bit 4: `RLHT_CAP_TC_SELECT`
- bit 5: `RLHT_CAP_OPEN_DUTY_CONTROL` — `SET_OPEN_DUTY` takes a duty per
  heater, 0 to `RLHT_OPEN_DUTY_MAX_PCT` (100) percent of the relay period
  inclusive. Firmware advertising `RLHT_CAP_REFUSES` leaves a duty above that
  unchanged and refuses with `BREAD_REFUSE_UNDEFINED_VALUE`; older firmware
  clamps it
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
- bit 8: `RLHT_CAP_ESTOP_COUNT` — the GET_STATE reply appends
  `[estop_count:u8]` after its 19 bytes (`RLHT_STATE_OFF_ESTOP_COUNT`):
  hardware e-stop presses since boot, wrapping at 255. On firmware without
  the flag the reply is 19 bytes and `rlht_parse_state_payload()` reports
  `estop_count` 0, which a controller must not read as a count
- bit 9: `RLHT_CAP_REFUSES` — a SET the firmware does not act on in full
  is refused with `crumbs_refuse()` and a `BREAD_REFUSE_*` reason
  (`bread_refuse.h`), so GET_ACK's applied means every part was acted on as
  sent. A frame acted on in part applies what it can and is refused with
  the reason of its first ignored part in payload order; a SET_TC_SELECT
  with one channel other than 1 or 2 sets the other channel and is refused
  with `BREAD_REFUSE_UNDEFINED_VALUE`. After a refusal the controller reads
  GET_STATE to see what took effect. On firmware without the flag every
  frame reports applied, including ones the firmware ignored

Baseline flag set:

- `RLHT_CAP_BASELINE_FLAGS` (all currently supported RLHT control features;
  does not include `RLHT_CAP_CMD_WATCHDOG`, `RLHT_CAP_CLEAR_WATCHDOG_TRIP`,
  `RLHT_CAP_ESTOP_COUNT` or `RLHT_CAP_REFUSES`)

## Extension Policy

Optional features are capability flags; the module major is the
wire-compatibility number (`protocol-versioning.md`). A new flag is
defined so that its absence means the behaviour before the flag existed,
because old firmware advertises no new bit. Controllers gate features on
flags, never on the module minor or patch.

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
