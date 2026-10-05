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

## Error Handling Expectations

Controllers should:

- report unsupported commands clearly,
- avoid sending commands behind missing capability flags,
- tolerate unknown capability flags.
