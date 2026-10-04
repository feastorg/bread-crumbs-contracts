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
from .bread_version_helpers import BREAD_OP_GET_VERSION
from .bread_watchdog import BREAD_OP_GET_WATCHDOG

__all__ = [
    "RLHT_CAP_BASELINE_FLAGS",
    "RLHT_CAP_CMD_WATCHDOG",
    "RLHT_CAP_LEVEL_1",
    "RLHT_CAP_LEVEL_2",
    "RLHT_CAP_LEVEL_3",
    "RLHT_CAP_MODE_CONTROL",
    "RLHT_CAP_OPEN_DUTY_CONTROL",
    "RLHT_CAP_PERIOD_CONTROL",
    "RLHT_CAP_PID_TUNING",
    "RLHT_CAP_SETPOINT_CONTROL",
    "RLHT_CAP_TC_SELECT",
    "RLHT_FLAG_ESTOP",
    "RLHT_FLAG_RELAY1_ON",
    "RLHT_FLAG_RELAY2_ON",
    "RLHT_MODE_CLOSED_LOOP",
    "RLHT_MODE_OPEN_LOOP",
    "RLHT_MODULE_VER_MAJOR",
    "RLHT_MODULE_VER_MINOR",
    "RLHT_MODULE_VER_PATCH",
    "RLHT_OP_GET_STATE",
    "RLHT_OP_SET_MODE",
    "RLHT_OP_SET_OPEN_DUTY",
    "RLHT_OP_SET_PERIODS",
    "RLHT_OP_SET_PID",
    "RLHT_OP_SET_SETPOINTS",
    "RLHT_OP_SET_TC_SELECT",
    "RLHT_STATE_FIXED_LEN",
    "RLHT_TYPE_ID",
    "RlhtStateResult",
    "rlht_parse_state_payload",
    "rlht_query_caps",
    "rlht_query_state",
    "rlht_query_version",
    "rlht_query_watchdog",
    "rlht_send_set_mode",
    "rlht_send_set_open_duty",
    "rlht_send_set_periods",
    "rlht_send_set_pid_x10",
    "rlht_send_set_setpoints",
    "rlht_send_set_tc_select",
    "rlht_send_set_watchdog",
]

RLHT_TYPE_ID = 0x01

RLHT_MODULE_VER_MAJOR = 1
RLHT_MODULE_VER_MINOR = 0
RLHT_MODULE_VER_PATCH = 0

RLHT_MODE_CLOSED_LOOP = 0x00
RLHT_MODE_OPEN_LOOP = 0x01

RLHT_OP_SET_MODE = 0x01
RLHT_OP_SET_SETPOINTS = 0x02
RLHT_OP_SET_PID = 0x03
RLHT_OP_SET_PERIODS = 0x04
RLHT_OP_SET_TC_SELECT = 0x05
RLHT_OP_SET_OPEN_DUTY = 0x06

RLHT_OP_GET_STATE = 0x80

RLHT_FLAG_ESTOP = 0x01
RLHT_FLAG_RELAY1_ON = 0x02
RLHT_FLAG_RELAY2_ON = 0x04

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
#: field by field up to offset 18.
RLHT_STATE_FIXED_LEN = 19

_STATE = struct.Struct("<BBhhhhHHHHB")
assert _STATE.size == RLHT_STATE_FIXED_LEN

_U8 = struct.Struct("<B")
_U8_U8 = struct.Struct("<BB")
_U16 = struct.Struct("<H")
_U16_U16 = struct.Struct("<HH")
_I16_I16 = struct.Struct("<hh")
_PID = struct.Struct("<BBBBBB")


@dataclass(frozen=True, slots=True)
class RlhtStateResult:
    """``rlht_state_result_t``."""

    mode: int
    flags: int
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


def rlht_query_state() -> bytes:
    """SET_REPLY payload requesting ``RLHT_OP_GET_STATE``."""
    return _U8.pack(RLHT_OP_GET_STATE)


def rlht_query_version() -> bytes:
    """SET_REPLY payload requesting the version reply."""
    return _U8.pack(BREAD_OP_GET_VERSION)


def rlht_query_caps() -> bytes:
    """SET_REPLY payload requesting ``BREAD_OP_GET_CAPS``."""
    return _U8.pack(BREAD_OP_GET_CAPS)


def rlht_query_watchdog() -> bytes:
    """SET_REPLY payload requesting ``BREAD_OP_GET_WATCHDOG``."""
    return _U8.pack(BREAD_OP_GET_WATCHDOG)


def rlht_parse_state_payload(payload: bytes) -> RlhtStateResult:
    """Parse a GET_STATE reply payload (``rlht_parse_state_payload``).

    Like the C parser, this reads the fixed 19-byte prefix and ignores any
    trailing bytes; a shorter payload is rejected.
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
    )
