# Controller Compatibility

## Required Runtime Flow

For each discovered device:

1. Query version (`opcode 0x00`).
2. Query capabilities (`BREAD_OP_GET_CAPS`).
3. Gate optional commands by returned flags.
4. Use baseline fallback if caps query is unavailable.

## Baseline Fallback Policy

If `GET_CAPS` fails:

- map by type ID to baseline capability set,
- disable optional/advanced operations,
- continue only with safe baseline operations.

## Compatibility Matrix (Current)

| Device Profile | Expected Cap Level |
| --- | --- |
| DCMT Gen1 on classic Nano | DCMT level 2 |
| DCMT Gen2 on classic Nano | DCMT level 2 |
| DCMT Gen1 on Nano Every | DCMT level 3 |
| DCMT Gen2 on Nano Every | DCMT level 3 |
| RLHT Gen1 firmware | RLHT level 1 |
| RLHT Gen2 firmware | RLHT level 1 |

Generation mapping is informational only; runtime caps remain authoritative.

## DCMT Module Major 2

Contracts 0.7.0 changed the DCMT `SET_PID` layout and set
`DCMT_MODULE_VER_MAJOR` to 2. A controller built against 0.7.0 or later
refuses DCMT firmware that reports major 1, and one built against an
earlier release refuses firmware that reports major 2
(`bread_check_module_compat()`), so neither sends a `SET_PID` the other
reads in the wrong layout. Update the DCMT firmware and the controller in
the same step.

## Command Watchdog Trip

What clears a latched watchdog trip (`tripped` in the `BREAD_OP_GET_WATCHDOG`
reply) depends on the firmware, and the caps flags say which:

| Flags advertised | `SET_WATCHDOG` clears the trip | `CLEAR_WATCHDOG_TRIP` |
| --- | --- | --- |
| `*_CAP_CMD_WATCHDOG` only | yes (so does any other valid command frame, not a query) | no handler, but the frame clears the trip like any command frame; do not send |
| `*_CAP_CMD_WATCHDOG` and `*_CAP_CLEAR_WATCHDOG_TRIP` | no | clears it |

On firmware with `*_CAP_CLEAR_WATCHDOG_TRIP`, a controller may re-arm with
`BREAD_OP_SET_WATCHDOG` at any time without releasing a hold. Send
`BREAD_OP_CLEAR_WATCHDOG_TRIP` only on an explicit operator request, never
as part of startup, address recovery or re-arming. On firmware without the
flag, re-arming also clears the trip, so a controller cannot keep a trip
latched there.

A trip latches on firmware with the flag even for a controller that does
not know the flag. Such a controller (anolis-provider-bread releases that
predate it, for example) has no remote way to clear the trip: SET_WATCHDOG
no longer does, and it does not send CLEAR_WATCHDOG_TRIP. Only a reboot or
a deliberate local operator command on the slice clears it. Ship the
controller release that understands the flag with or before firmware that
advertises it.

Firmware rejects a `BREAD_OP_CLEAR_WATCHDOG_TRIP` frame with a non-empty
payload and leaves the trip set.

## DCMT E-stop Hold

On DCMT firmware that advertises `DCMT_CAP_ESTOP_HOLD`, a hardware e-stop
leaves the board as a latched watchdog trip does: both brake flags set
(visible in `brakes` in GET_STATE) and the position setpoints at the
encoders. Motion commands and brake releases sent while `estop` is set
are refused. After the release nothing moves until the controller sends
`SET_BRAKE(0,0)`, then setpoints, then the mode, the same sequence that
resumes after a watchdog trip is cleared.

On firmware without the flag, releasing the e-stop resumes the stored
mode and setpoints, and commands sent during the hold take effect at the
release. A controller that wants a release to move nothing sends its safe
state while `estop` is set, and cannot act on a press it never polled.

## Error Handling Expectations

Controllers should:

- report unsupported commands clearly,
- avoid sending commands behind missing capability flags,
- tolerate unknown capability flags.
