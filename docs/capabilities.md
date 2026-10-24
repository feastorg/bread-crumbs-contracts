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
  moves the position setpoints to the encoders. While e-stopped it refuses
  `SET_OPEN_LOOP`, `SET_SETPOINT`, `SET_MODE` and any brake release with
  `BREAD_REFUSE_ESTOP`. Each motor's position setpoint keeps following its
  encoder until that motor's brake is released or a new setpoint for it
  arrives, through the e-stop release and after a watchdog trip is
  cleared, so the brake release holds where the shaft is; a setpoint the
  controller sends stands as sent. Releasing the e-stop moves nothing; the
  controller resumes with `SET_BRAKE(0,0)`, then setpoints, then the mode.
  Advertised only together with `DCMT_CAP_REFUSES`. On firmware without
  the flag, a release resumes the stored mode and setpoints, including
  commands sent during the hold
- bit 8: `DCMT_CAP_REFUSES` — a SET the firmware does not act on in full
  is refused with `crumbs_refuse()` and a `BREAD_REFUSE_*` reason
  (`bread_refuse.h`), so GET_ACK's applied means every part was acted on as
  sent. A frame acted on in part applies what it can and is refused with
  the reason of its first ignored part in payload order; a SET_BRAKE while
  a hold is in force engages any brake it names and is refused with the
  hold's reason (`BREAD_REFUSE_WATCHDOG_TRIP`, or `BREAD_REFUSE_ESTOP` on
  firmware with `DCMT_CAP_ESTOP_HOLD`) if it asked for a release. After a refusal
  the controller reads GET_STATE to see what took effect. On firmware
  without the flag every frame reports applied, including ones the
  firmware ignored
- bit 9: `DCMT_CAP_SETPOINT_LOOP` — `SET_SETPOINT` takes the 5-byte form
  `[loop:u8][target1:i16][target2:i16]`, which writes only the named
  loop's setpoints (`DCMT_PID_LOOP_*`), and the 4-byte form writes the
  position setpoints only. Any other length is refused
  `BREAD_REFUSE_MALFORMED`, an undefined loop `BREAD_REFUSE_UNDEFINED_VALUE`.
  A switch into closed-loop speed starts from the speed setpoint, never a
  position target. Advertised only together with `DCMT_CAP_REFUSES`. On
  firmware without the flag the 4-byte form writes both
- bit 10: `DCMT_CAP_WATCHDOG_LEASE` — `SET_WATCHDOG` takes the 3-byte lease
  form (`bread_watchdog.h`). With `BREAD_WATCHDOG_FLAG_COMMANDS_ONLY` only
  `SET_OPEN_LOOP`, `SET_BRAKE`, `SET_MODE`, `SET_SETPOINT` and
  `SET_WATCHDOG` refresh the watchdog. Advertised only together with
  `DCMT_CAP_CMD_WATCHDOG`
- bit 11: `DCMT_CAP_FAULTS` — stall faults latch per motor (`bread_faults.h`):
  `BREAD_OP_GET_FAULTS` reports `DCMT_FAULT_*` and `BREAD_OP_CLEAR_FAULTS`
  clears them. While any is latched both motors are braked, and
  `SET_OPEN_LOOP`, `SET_SETPOINT`, `SET_MODE` and any brake release are
  refused `BREAD_REFUSE_FAULT`. Advertised only together with
  `DCMT_CAP_REFUSES`

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
- bit 10: `RLHT_CAP_ESTOP_HOLD` — a hardware e-stop holds the way a latched
  watchdog trip does. On a press the firmware sets both setpoints and both
  open-loop on-times to 0 and turns the relays off. While e-stopped it
  refuses `SET_MODE`, `SET_SETPOINTS` and `SET_OPEN_DUTY` with
  `BREAD_REFUSE_ESTOP`. Releasing the e-stop heats nothing; the controller
  resumes with setpoints or duties. Advertised only together with
  `RLHT_CAP_REFUSES`. On firmware without the flag, commands sent during
  the hold are stored and heat at the release
- bit 11: `RLHT_CAP_WATCHDOG_LEASE` — `SET_WATCHDOG` takes the 3-byte lease
  form (`bread_watchdog.h`). With `BREAD_WATCHDOG_FLAG_COMMANDS_ONLY` only
  `SET_MODE`, `SET_SETPOINTS`, `SET_OPEN_DUTY` and `SET_WATCHDOG` refresh
  the watchdog. Advertised only together with `RLHT_CAP_CMD_WATCHDOG`
- bit 12: `RLHT_CAP_PID_X1000` — `SET_PID` takes the 12-byte form, six u16
  gains x1000, beside the 6-byte u8 x10 one, and refuses any other length
  `BREAD_REFUSE_MALFORMED`
- bit 13: `RLHT_CAP_FAULTS` — heater faults latch (`bread_faults.h`):
  over-temperature in either mode, no rise at near-full output in closed
  loop, and an open thermocouple. `BREAD_OP_GET_FAULTS` reports
  `RLHT_FAULT_*`, GET_STATE sets `RLHT_FLAG_FAULT`, and
  `BREAD_OP_CLEAR_FAULTS` clears them. While any is latched both heaters
  are off, and `SET_MODE`, `SET_SETPOINTS` and `SET_OPEN_DUTY` are refused
  `BREAD_REFUSE_FAULT`. Thresholds are the firmware's build-time settings.
  Advertised only together with `RLHT_CAP_REFUSES`
- bit 14: `RLHT_CAP_SETPOINT_LIMITS` — `SET_SETPOINTS` is refused
  `BREAD_REFUSE_WRONG_MODE` in open loop and `BREAD_REFUSE_UNDEFINED_VALUE`
  above the firmware's ceiling (Slice_RLHT defaults to 500 °C), storing
  neither. Advertised only together with `RLHT_CAP_REFUSES`. On firmware
  without the flag any value is stored in either mode, and one sent in open
  loop heats at the next switch to closed loop

Baseline flag set:

- `RLHT_CAP_BASELINE_FLAGS` (all currently supported RLHT control features;
  does not include `RLHT_CAP_CMD_WATCHDOG`, `RLHT_CAP_CLEAR_WATCHDOG_TRIP`,
  `RLHT_CAP_ESTOP_COUNT`, `RLHT_CAP_REFUSES`, `RLHT_CAP_ESTOP_HOLD` or any
  later flag)

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

Four exceptions change what a controller that predates the flag sees, so
the controller release that understands the flag must ship with or before
such firmware (see `controller-compatibility.md`):

- Firmware that advertises `DCMT_CAP_CLEAR_WATCHDOG_TRIP` /
  `RLHT_CAP_CLEAR_WATCHDOG_TRIP` changes what `BREAD_OP_SET_WATCHDOG` does,
  because it no longer clears a watchdog trip. A controller that predates
  the flag has no remote way to clear a trip on that firmware.
- Firmware that advertises `DCMT_CAP_ESTOP_HOLD` / `RLHT_CAP_ESTOP_HOLD`
  refuses, while the e-stop is held, commands it used to store and run at
  the release, and a DCMT stays braked after the release until
  `SET_BRAKE(0,0)`. On DCMT the position setpoints also keep following the
  encoders after a watchdog trip is cleared, until each brake is released
  or a new setpoint arrives, where they used to stop following at the
  clear. All three fail safe, but a controller that predates the flag does
  not expect them.
- Firmware that advertises `DCMT_CAP_SETPOINT_LOOP` writes the 4-byte
  `SET_SETPOINT` into the position setpoints only. A controller that
  predates the flag and relied on it to set a speed target leaves the speed
  setpoint where it was, so closed-loop speed runs at that target (0 after
  a hold), not at the value it sent.
- Firmware that advertises `RLHT_CAP_SETPOINT_LIMITS` refuses
  `SET_SETPOINTS` in open loop and above its ceiling, where it used to
  store them. A controller that predates the flag sees a refusal, and
  nothing heats from a stored value.
