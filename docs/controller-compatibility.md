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

Contracts 0.7.0 changed the DCMT `SET_PID` layout, appended `estop_count`
to the DCMT GET_STATE reply (20 bytes) and set
`DCMT_MODULE_VER_MAJOR` to 2. A controller built against 0.7.0 or later
refuses DCMT firmware that reports major 1, and one built against an
earlier release refuses firmware that reports major 2
(`bread_check_module_compat()`), so neither sends a `SET_PID` the other
reads in the wrong layout. Update the DCMT firmware and the controller in
the same step:

- the controller moves its contracts pin to 0.7.0 and sends the new
  `SET_PID` (for anolis-provider-bread, anolishq/anolis-provider-bread#147);
- every DCMT board is reflashed with firmware built against `^0.7.0`. A
  `^0.6.0` pin, which Slice_DCMT's `platformio.ini` carries today, does
  not resolve 0.7.0 on 0.x, so the pin changes first;
- until both sides move, the controller refuses the board: a 0.7.0
  controller refuses a board that was not reflashed (anolis-provider-bread
  reports `incompatible_module_major`), and an older controller refuses a
  reflashed one;
- RLHT boards need no reflash. RLHT stays major 1 and is wire compatible
  both ways; only `estop_count` needs RLHT firmware that advertises
  `RLHT_CAP_ESTOP_COUNT`.

## Command Watchdog Trip

What refreshes the watchdog is listed in `include/bread/bread_watchdog.h`. It
counts reads as well as commands: GET_STATE, GET_CAPS, GET_WATCHDOG and the
version read all refresh it, and so does any bare I2C read while one of them
is staged. A second reader on the bus therefore keeps a slice's watchdog fed
after the controller that armed it has stopped.

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

Clearing a trip resumes nothing. To resume on DCMT, send `SET_BRAKE(0,0)`,
then setpoints, then the mode; on RLHT, send setpoints or duties. On DCMT
firmware with `DCMT_CAP_ESTOP_HOLD`, each position setpoint keeps following
its encoder after the clear until that motor's brake is released or a new
setpoint for it arrives, as after an e-stop release.

## DCMT E-stop Hold

On DCMT firmware that advertises `DCMT_CAP_ESTOP_HOLD`, a hardware e-stop
leaves the board as a latched watchdog trip does: both brake flags set
(visible in `brakes` in GET_STATE) and the position setpoints at the
encoders. Motion commands and brake releases sent while `estop` is set
are refused with `BREAD_REFUSE_ESTOP`. After the release nothing moves
until the controller sends `SET_BRAKE(0,0)`, then setpoints, then the
mode, the same sequence that resumes after a watchdog trip is cleared.
Until each brake is released or a new setpoint for that motor arrives, its
position setpoint keeps following its encoder, so a brake release with no
new setpoint holds where the shaft is. A setpoint sent before
`SET_BRAKE(0,0)` ends the follow and stands as sent, so the brake release
drives to it.

On firmware without the flag, releasing the e-stop resumes the stored
mode and setpoints, and commands sent during the hold take effect at the
release. A controller that wants a release to move nothing sends its safe
state while `estop` is set, and cannot act on a press it never polled.

## RLHT E-stop Hold

On RLHT firmware that advertises `RLHT_CAP_ESTOP_HOLD`, a hardware e-stop
leaves the board as a latched watchdog trip does: both setpoints and both
open-loop on-times at 0 and the relays off. `SET_MODE`, `SET_SETPOINTS` and
`SET_OPEN_DUTY` sent while `estop` is set are refused with
`BREAD_REFUSE_ESTOP`. After the release nothing heats until the controller
sends setpoints or duties.

On current firmware without the flag, the press zeroes the same fields, but
setpoints, duties and modes sent during the hold are stored and take
effect at the release. Older RLHT firmware may also carry its PID state
across the hold, so heating can resume from where it was once a setpoint
returns.

## E-stop Wiring

A slice's e-stop may be wired two ways, and with `*_CAP_ESTOP_HOLD` both
end the same way: after a stop, nothing moves or heats until the controller
sends new commands.

- **Signal line.** The e-stop pulls the slice's e-stop input low. The
  firmware holds as above, `estop` reads 1 in GET_STATE and `estop_count`
  advances (on RLHT, with `RLHT_CAP_ESTOP_COUNT`). Some slice hardware may
  also cut actuator power from the same line; on hardware without that
  cut, the firmware hold is the only stop. Check the slice's documentation
  or schematic before relying on a hardware cut.
- **Power cut.** The e-stop removes the supply. The slice reboots into its
  boot state, which moves and heats nothing: DCMT in open loop at PWM 0,
  RLHT at setpoints 0. The firmware sees no press. A slice whose logic stays
  powered from another source, such as USB, sees no stop at all and runs its
  stored commands when actuator power returns, so power the logic from the
  switched supply.

A controller sees a power-cut stop only as a reboot. The most dependable
sign, for a controller that armed the command watchdog, is
`BREAD_OP_GET_WATCHDOG` back in its boot state: disarmed, unless the
firmware is built with `*_WATCHDOG_BOOT_MS`. `estop_count` also restarts
from 0, or from 1 if a signal-wired line is still held at power-up, and
the slice stops answering for the length of the cut. The controller must
not replay commands it sent before the reboot without an operator's
go-ahead.

## E-stop Count

`estop_count` in GET_STATE counts hardware e-stop presses since boot and
wraps at 255. A controller compares it with the previous poll, modulo 256,
to see presses that started and ended between polls; a reboot resets it
to 0. DCMT major 2 always carries it. RLHT carries it when
`RLHT_CAP_ESTOP_COUNT` is advertised; without the flag, ignore the parsed
value.

## Refused Commands

CRUMBS 0.16.0's GET_ACK says whether the peripheral applied, refused or
rejected the last command. A slice that declines a SET refuses it with one
of the `BREAD_REFUSE_*` reasons in `bread_refuse.h`. What a controller does
with each:

- `BREAD_REFUSE_MALFORMED`, `BREAD_REFUSE_UNDEFINED_VALUE`,
  `BREAD_REFUSE_UNKNOWN_OP`: the controller built a frame the contract does
  not allow. Resending it will not help. A GET opcode is a query: read it
  through SET_REPLY. Sent as a command frame, it is refused with
  `BREAD_REFUSE_UNKNOWN_OP`.

A slice that declares its family's `type_id` to CRUMBS rejects a command
frame of another type before acting on it, and GET_ACK reports it rejected
(`CRUMBS_ACK_REJECT_TYPE`), not refused. Send commands with the family's
`type_id`, as the builders here do, or `0x00`.
- `BREAD_REFUSE_NOT_BUILT`: the command needs a feature this build leaves
  out. Gate it on the capability flag that names the feature.
- `BREAD_REFUSE_WATCHDOG_TRIP`, `BREAD_REFUSE_ESTOP`: the slice is holding
  its safe state. Clear the trip, or wait for the e-stop release, before
  resending. On firmware with `*_CAP_ESTOP_HOLD`, with both held the
  reason is `BREAD_REFUSE_ESTOP`, since clearing the trip does not lift the
  e-stop hold; firmware without the flag never refuses with
  `BREAD_REFUSE_ESTOP`. A command that only asks
  for the safe value, such as `SET_OPEN_LOOP(0,0)` or `SET_SETPOINTS` 0/0,
  is refused the same way: the slice is already at least that safe, so do
  not retry it.
- `BREAD_REFUSE_WRONG_MODE`: set the mode the command needs first.

A reason from `0x80` up is the family's own; the family header defines it.

Applied means every part of the frame was acted on as sent. Refused does
not mean nothing changed. A frame the firmware can act on in part applies
those parts and is refused with the reason of its first ignored part, in
payload order:

- DCMT SET_BRAKE while a hold is in force engages any brake it names,
  since an engage is always safe, and is refused with the hold's reason
  (`BREAD_REFUSE_WATCHDOG_TRIP`, or `BREAD_REFUSE_ESTOP` on firmware with
  `DCMT_CAP_ESTOP_HOLD`) if it asked for a release.
- RLHT SET_TC_SELECT with one channel other than 1 or 2 sets the other
  channel and is refused with `BREAD_REFUSE_UNDEFINED_VALUE`.
- A field outside its stated range is a value the contract does not
  define, so it is left unchanged and the frame is refused with
  `BREAD_REFUSE_UNDEFINED_VALUE`, while the in-range field beside it still
  applies. The ranges are inclusive: RLHT SET_OPEN_DUTY 0 to
  `RLHT_OPEN_DUTY_MAX_PCT` (100) percent, RLHT SET_PERIODS
  `RLHT_PERIOD_MIN_MS` (100) to `RLHT_PERIOD_MAX_MS` (10000) ms, DCMT
  SET_OPEN_LOOP `-DCMT_PWM_MAX` to `DCMT_PWM_MAX` (255). Firmware without
  the REFUSES flag clamps such a value instead. Check the range before
  sending; the codec's encoders check only the wire type.

After any refusal, read GET_STATE to see what took effect before resending.

Trust "applied" only when GET_CAPS advertises `DCMT_CAP_REFUSES` or
`RLHT_CAP_REFUSES`. Firmware without the flag never refuses: it dispatches
every frame through `on_message`, so GET_ACK reports each one applied,
including commands the firmware ignored. On such firmware, "applied" means
only that the frame arrived, and a controller confirms the effect in
GET_STATE.

## Error Handling Expectations

Controllers should:

- report unsupported commands clearly,
- avoid sending commands behind missing capability flags,
- tolerate unknown capability flags.
