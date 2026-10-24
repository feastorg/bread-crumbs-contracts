"""RLHT (relay heater) contract (``rlht_ops.h``).

Each ``rlht_send_*`` function returns the payload its C namesake puts on
the wire; send it with ``RLHT_TYPE_ID`` and the matching ``RLHT_OP_*``
opcode. Each ``rlht_query_*`` function returns the one-byte payload of the
SET_REPLY frame (type ``CRUMBS_TYPE_ID_ANY``, opcode ``CRUMBS_CMD_SET_REPLY``)
that asks the slice to build that reply.
"""

import struct
from dataclasses import dataclass

from ._wire import i16, require_min_len, u8, u16
from .bread_caps import BREAD_OP_GET_CAPS
from .bread_faults import BREAD_OP_GET_FAULTS
from .bread_version_helpers import BREAD_OP_GET_VERSION
from .bread_watchdog import BREAD_OP_GET_WATCHDOG

__all__ = [
    "RLHT_CAP_BASELINE_FLAGS",
    "RLHT_CAP_CLEAR_WATCHDOG_TRIP",
    "RLHT_CAP_ESTOP_COUNT",
    "RLHT_CAP_ESTOP_HOLD",
    "RLHT_CAP_FAULTS",
    "RLHT_CAP_CMD_WATCHDOG",
    "RLHT_CAP_LEVEL_1",
    "RLHT_CAP_LEVEL_2",
    "RLHT_CAP_LEVEL_3",
    "RLHT_CAP_MODE_CONTROL",
    "RLHT_CAP_OPEN_DUTY_CONTROL",
    "RLHT_CAP_PERIOD_CONTROL",
    "RLHT_CAP_PID_TUNING",
    "RLHT_CAP_PID_X1000",
    "RLHT_CAP_REFUSES",
    "RLHT_CAP_SETPOINT_CONTROL",
    "RLHT_CAP_SETPOINT_LIMITS",
    "RLHT_CAP_TC_SELECT",
    "RLHT_CAP_WATCHDOG_LEASE",
    "RLHT_FAULT_H1_NO_RISE",
    "RLHT_FAULT_H1_OVERTEMP",
    "RLHT_FAULT_H1_SENSOR",
    "RLHT_FAULT_H2_NO_RISE",
    "RLHT_FAULT_H2_OVERTEMP",
    "RLHT_FAULT_H2_SENSOR",
    "RLHT_FLAG_ESTOP",
    "RLHT_FLAG_FAULT",
    "RLHT_FLAG_RELAY1_ON",
    "RLHT_FLAG_RELAY2_ON",
    "RLHT_MODE_CLOSED_LOOP",
    "RLHT_MODE_OPEN_LOOP",
    "RLHT_MODULE_VER_MAJOR",
    "RLHT_MODULE_VER_MINOR",
    "RLHT_MODULE_VER_PATCH",
    "RLHT_OPEN_DUTY_MAX_PCT",
    "RLHT_OP_GET_STATE",
    "RLHT_OP_SET_MODE",
    "RLHT_OP_SET_OPEN_DUTY",
    "RLHT_OP_SET_PERIODS",
    "RLHT_OP_SET_PID",
    "RLHT_OP_SET_SETPOINTS",
    "RLHT_OP_SET_TC_SELECT",
    "RLHT_PERIOD_MAX_MS",
    "RLHT_PERIOD_MIN_MS",
    "RLHT_SET_PID_PAYLOAD_LEN",
    "RLHT_SET_PID_X1000_PAYLOAD_LEN",
    "RLHT_STATE_FIXED_LEN",
    "RLHT_STATE_OFF_ESTOP_COUNT",
    "RLHT_TYPE_ID",
    "RlhtStateResult",
    "rlht_parse_state_payload",
    "rlht_query_caps",
    "rlht_query_faults",
    "rlht_query_state",
    "rlht_query_version",
    "rlht_query_watchdog",
    "rlht_send_clear_faults",
    "rlht_send_clear_watchdog_trip",
    "rlht_send_set_mode",
    "rlht_send_set_open_duty",
    "rlht_send_set_periods",
    "rlht_send_set_pid_x10",
    "rlht_send_set_pid_x1000",
    "rlht_send_set_setpoints",
    "rlht_send_set_tc_select",
    "rlht_send_set_watchdog",
    "rlht_send_set_watchdog_lease",
]

RLHT_TYPE_ID = 0x01

RLHT_MODULE_VER_MAJOR = 1
RLHT_MODULE_VER_MINOR = 0
RLHT_MODULE_VER_PATCH = 0

RLHT_MODE_CLOSED_LOOP = 0x00
RLHT_MODE_OPEN_LOOP = 0x01

#: Field ranges, inclusive. Firmware advertising ``RLHT_CAP_REFUSES`` leaves a
#: value outside its range unchanged and refuses with
#: ``BREAD_REFUSE_UNDEFINED_VALUE``; older firmware clamps it. The encoders
#: check only the wire type, so a value outside these still encodes.
RLHT_PERIOD_MIN_MS = 100
RLHT_PERIOD_MAX_MS = 10000
RLHT_OPEN_DUTY_MAX_PCT = 100

RLHT_OP_SET_MODE = 0x01
RLHT_OP_SET_SETPOINTS = 0x02
RLHT_OP_SET_PID = 0x03
RLHT_OP_SET_PERIODS = 0x04
RLHT_OP_SET_TC_SELECT = 0x05
RLHT_OP_SET_OPEN_DUTY = 0x06

RLHT_OP_GET_STATE = 0x80

#: SET_PID: six ``u8`` gains x10 (6 bytes), or, on firmware advertising
#: ``RLHT_CAP_PID_X1000``, six ``u16`` gains x1000 (12 bytes). Such firmware
#: refuses any other length; send the 12-byte form only when it is advertised.
RLHT_SET_PID_PAYLOAD_LEN = 6
RLHT_SET_PID_X1000_PAYLOAD_LEN = 12

RLHT_FLAG_ESTOP = 0x01
RLHT_FLAG_RELAY1_ON = 0x02
RLHT_FLAG_RELAY2_ON = 0x04
#: A fault is latched (``RLHT_CAP_FAULTS``); GET_FAULTS says which.
RLHT_FLAG_FAULT = 0x08

RLHT_CAP_LEVEL_1 = 0x01
RLHT_CAP_LEVEL_2 = 0x02
RLHT_CAP_LEVEL_3 = 0x03

RLHT_CAP_MODE_CONTROL = 1 << 0
RLHT_CAP_SETPOINT_CONTROL = 1 << 1
RLHT_CAP_PID_TUNING = 1 << 2
RLHT_CAP_PERIOD_CONTROL = 1 << 3
RLHT_CAP_TC_SELECT = 1 << 4
RLHT_CAP_OPEN_DUTY_CONTROL = 1 << 5
RLHT_CAP_CMD_WATCHDOG = 1 << 6
#: The watchdog trip latches until CLEAR_WATCHDOG_TRIP, a deliberate local
#: operator command or reboot; SET_WATCHDOG no longer clears it. Without
#: this flag, SET_WATCHDOG clears the trip.
RLHT_CAP_CLEAR_WATCHDOG_TRIP = 1 << 7
#: The GET_STATE reply carries ``estop_count`` after the 19-byte layout.
#: Without it the reply is 19 bytes and ``estop_count`` parses as 0.
RLHT_CAP_ESTOP_COUNT = 1 << 8
#: A SET the firmware does not act on in full is refused with a
#: ``BREAD_REFUSE_*`` reason, so GET_ACK's applied means every part was acted
#: on as sent; a frame acted on in part applies what it can and is refused.
#: Without this flag every frame reports applied, including ones the firmware
#: ignored.
RLHT_CAP_REFUSES = 1 << 9
#: A hardware e-stop holds like a latched watchdog trip: setpoints and
#: open-loop on-times at 0, relays off, and SET_MODE, SET_SETPOINTS and
#: SET_OPEN_DUTY refused while e-stopped; a release heats nothing.
#: Advertised only together with ``RLHT_CAP_REFUSES``. Without this flag,
#: commands sent during the hold are stored and heat at the release.
RLHT_CAP_ESTOP_HOLD = 1 << 10
#: SET_WATCHDOG takes the 3-byte lease form (``bread_watchdog``).
RLHT_CAP_WATCHDOG_LEASE = 1 << 11
#: SET_PID takes the 12-byte ``u16`` x1000 form beside the 6-byte one.
RLHT_CAP_PID_X1000 = 1 << 12
#: Heater faults latch (over-temperature, no rise at near-full output, open
#: thermocouple): GET_FAULTS reports ``RLHT_FAULT_*`` bits, GET_STATE sets
#: ``RLHT_FLAG_FAULT``, CLEAR_FAULTS clears them; while any is latched both
#: heaters are off and heating commands are refused with ``BREAD_REFUSE_FAULT``.
RLHT_CAP_FAULTS = 1 << 13
#: SET_SETPOINTS is refused in open loop (``BREAD_REFUSE_WRONG_MODE``) and
#: above the firmware's ceiling (``BREAD_REFUSE_UNDEFINED_VALUE``). Without it,
#: any value is stored in either mode.
RLHT_CAP_SETPOINT_LIMITS = 1 << 14

#: GET_FAULTS bits, per heater.
RLHT_FAULT_H1_OVERTEMP = 0x01
RLHT_FAULT_H1_NO_RISE = 0x02
RLHT_FAULT_H1_SENSOR = 0x04
RLHT_FAULT_H2_OVERTEMP = 0x08
RLHT_FAULT_H2_NO_RISE = 0x10
RLHT_FAULT_H2_SENSOR = 0x20

RLHT_CAP_BASELINE_FLAGS = (
    RLHT_CAP_MODE_CONTROL
    | RLHT_CAP_SETPOINT_CONTROL
    | RLHT_CAP_PID_TUNING
    | RLHT_CAP_PERIOD_CONTROL
    | RLHT_CAP_TC_SELECT
    | RLHT_CAP_OPEN_DUTY_CONTROL
)

#: GET_STATE payload (19 bytes):
#: ``[mode:u8][flags:u8][t1:i16][t2:i16][sp1:i16][sp2:i16]``
#: ``[on1:u16][on2:u16][period1:u16][period2:u16][tc_select:u8]``.
#: ``rlht_ops.h`` has no length macro; ``rlht_parse_state_payload()`` reads
#: field by field up to offset 18. Firmware advertising
#: ``RLHT_CAP_ESTOP_COUNT`` appends ``[estop_count:u8]`` at offset 19: e-stop
#: presses since boot, wrapping at 255.
RLHT_STATE_FIXED_LEN = 19
RLHT_STATE_OFF_ESTOP_COUNT = 19

_STATE = struct.Struct("<BBhhhhHHHHB")
assert _STATE.size == RLHT_STATE_FIXED_LEN

_U8 = struct.Struct("<B")
_U8_U8 = struct.Struct("<BB")
_U16 = struct.Struct("<H")
_U16_U16 = struct.Struct("<HH")
_I16_I16 = struct.Struct("<hh")
_PID = struct.Struct("<BBBBBB")
assert _PID.size == RLHT_SET_PID_PAYLOAD_LEN
_PID_X1000 = struct.Struct("<HHHHHH")
assert _PID_X1000.size == RLHT_SET_PID_X1000_PAYLOAD_LEN
_U16_U8 = struct.Struct("<HB")


@dataclass(frozen=True, slots=True)
class RlhtStateResult:
    """``rlht_state_result_t``."""

    mode: int
    flags: int
    # BREAD_INVALID_I16 while that thermocouple reads open, as is t2_deci_c.
    t1_deci_c: int
    t2_deci_c: int
    sp1_deci_c: int
    sp2_deci_c: int
    on1_ms: int
    on2_ms: int
    period1_ms: int
    period2_ms: int
    tc_select: int
    #: Derived from ``tc_select``: bits 0-1 and bits 2-3.
    tc1: int
    tc2: int
    #: Byte 19 when the reply carries it, else 0; meaningful only when
    #: ``RLHT_CAP_ESTOP_COUNT`` is advertised.
    estop_count: int


def rlht_send_set_mode(mode: int) -> bytes:
    """``RLHT_OP_SET_MODE`` payload: ``[mode:u8]`` (``RLHT_MODE_*``)."""
    return _U8.pack(u8("mode", mode))


def rlht_send_set_setpoints(sp1_deci_c: int, sp2_deci_c: int) -> bytes:
    """``RLHT_OP_SET_SETPOINTS`` payload: ``[sp1:i16][sp2:i16]`` in deci-degrees C."""
    return _I16_I16.pack(i16("sp1_deci_c", sp1_deci_c), i16("sp2_deci_c", sp2_deci_c))


def rlht_send_set_pid_x10(
    kp1_x10: int, ki1_x10: int, kd1_x10: int, kp2_x10: int, ki2_x10: int, kd2_x10: int
) -> bytes:
    """``RLHT_OP_SET_PID`` payload: six ``u8`` gains, each ten times the gain."""
    return _PID.pack(
        u8("kp1_x10", kp1_x10),
        u8("ki1_x10", ki1_x10),
        u8("kd1_x10", kd1_x10),
        u8("kp2_x10", kp2_x10),
        u8("ki2_x10", ki2_x10),
        u8("kd2_x10", kd2_x10),
    )


def rlht_send_set_pid_x1000(
    kp1_x1000: int,
    ki1_x1000: int,
    kd1_x1000: int,
    kp2_x1000: int,
    ki2_x1000: int,
    kd2_x1000: int,
) -> bytes:
    """``RLHT_OP_SET_PID`` 12-byte payload: six ``u16`` gains, each a thousand
    times the gain.

    Send only when ``RLHT_CAP_PID_X1000`` is advertised.
    """
    return _PID_X1000.pack(
        u16("kp1_x1000", kp1_x1000),
        u16("ki1_x1000", ki1_x1000),
        u16("kd1_x1000", kd1_x1000),
        u16("kp2_x1000", kp2_x1000),
        u16("ki2_x1000", ki2_x1000),
        u16("kd2_x1000", kd2_x1000),
    )


def rlht_send_set_periods(p1_ms: int, p2_ms: int) -> bytes:
    """``RLHT_OP_SET_PERIODS`` payload: ``[p1:u16][p2:u16]`` in milliseconds."""
    return _U16_U16.pack(u16("p1_ms", p1_ms), u16("p2_ms", p2_ms))


def rlht_send_set_tc_select(tc1: int, tc2: int) -> bytes:
    """``RLHT_OP_SET_TC_SELECT`` payload: ``[tc1:u8][tc2:u8]``."""
    return _U8_U8.pack(u8("tc1", tc1), u8("tc2", tc2))


def rlht_send_set_open_duty(duty1_pct: int, duty2_pct: int) -> bytes:
    """``RLHT_OP_SET_OPEN_DUTY`` payload: ``[duty1:u8][duty2:u8]`` in percent."""
    return _U8_U8.pack(u8("duty1_pct", duty1_pct), u8("duty2_pct", duty2_pct))


def rlht_send_set_watchdog(timeout_ms: int) -> bytes:
    """``BREAD_OP_SET_WATCHDOG`` payload: ``[timeout_ms:u16]``, 0 = disarm."""
    return _U16.pack(u16("timeout_ms", timeout_ms))


def rlht_send_set_watchdog_lease(timeout_ms: int, flags: int) -> bytes:
    """``BREAD_OP_SET_WATCHDOG`` lease payload: ``[timeout_ms:u16][flags:u8]``.

    Send only when ``RLHT_CAP_WATCHDOG_LEASE`` is advertised.
    """
    return _U16_U8.pack(u16("timeout_ms", timeout_ms), u8("flags", flags))


def rlht_send_clear_faults() -> bytes:
    """``BREAD_OP_CLEAR_FAULTS`` payload: empty.

    Send only when ``RLHT_CAP_FAULTS`` is advertised, and only on an
    operator's request.
    """
    return b""


def rlht_send_clear_watchdog_trip() -> bytes:
    """``BREAD_OP_CLEAR_WATCHDOG_TRIP`` payload: empty.

    Send only when ``RLHT_CAP_CLEAR_WATCHDOG_TRIP`` is advertised, and only
    on an operator's request.
    """
    return b""


def rlht_query_state() -> bytes:
    """SET_REPLY payload requesting ``RLHT_OP_GET_STATE``."""
    return _U8.pack(RLHT_OP_GET_STATE)


def rlht_query_version() -> bytes:
    """SET_REPLY payload requesting the version reply."""
    return _U8.pack(BREAD_OP_GET_VERSION)


def rlht_query_caps() -> bytes:
    """SET_REPLY payload requesting ``BREAD_OP_GET_CAPS``."""
    return _U8.pack(BREAD_OP_GET_CAPS)


def rlht_query_faults() -> bytes:
    """SET_REPLY payload requesting ``BREAD_OP_GET_FAULTS``."""
    return _U8.pack(BREAD_OP_GET_FAULTS)


def rlht_query_watchdog() -> bytes:
    """SET_REPLY payload requesting ``BREAD_OP_GET_WATCHDOG``."""
    return _U8.pack(BREAD_OP_GET_WATCHDOG)


def rlht_parse_state_payload(payload: bytes) -> RlhtStateResult:
    """Parse a GET_STATE reply payload (``rlht_parse_state_payload``).

    Like the C parser, this reads the fixed 19-byte prefix, then
    ``estop_count`` from byte 19 if present (0 if not), and ignores any
    further bytes; a payload shorter than 19 bytes is rejected.
    """
    require_min_len("RLHT GET_STATE", payload, RLHT_STATE_FIXED_LEN)
    (mode, flags, t1, t2, sp1, sp2, on1, on2, period1, period2, tc_select) = _STATE.unpack_from(
        payload
    )
    return RlhtStateResult(
        mode=mode,
        flags=flags,
        t1_deci_c=t1,
        t2_deci_c=t2,
        sp1_deci_c=sp1,
        sp2_deci_c=sp2,
        on1_ms=on1,
        on2_ms=on2,
        period1_ms=period1,
        period2_ms=period2,
        tc_select=tc_select,
        tc1=tc_select & 0x03,
        tc2=(tc_select >> 2) & 0x03,
        estop_count=(
            payload[RLHT_STATE_OFF_ESTOP_COUNT] if len(payload) > RLHT_STATE_OFF_ESTOP_COUNT else 0
        ),
    )
