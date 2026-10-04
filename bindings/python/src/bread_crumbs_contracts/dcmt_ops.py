"""DCMT (DC motor) contract (``dcmt_ops.h``).

Each ``dcmt_send_*`` function returns the payload its C namesake puts on
the wire; send it with ``DCMT_TYPE_ID`` and the matching ``DCMT_OP_*``
opcode. Each ``dcmt_query_*`` function returns the one-byte payload of the
SET_REPLY frame (type ``CRUMBS_TYPE_ID_ANY``, opcode ``CRUMBS_CMD_SET_REPLY``)
that asks the slice to build that reply.
"""

import struct
from dataclasses import dataclass

from ._wire import i16, require_exact_len, u8, u16
from .bread_caps import BREAD_OP_GET_CAPS
from .bread_version_helpers import BREAD_OP_GET_VERSION
from .bread_watchdog import BREAD_OP_GET_WATCHDOG

__all__ = [
    "DCMT_CAP_BASELINE_FLAGS",
    "DCMT_CAP_BRAKE_CONTROL",
    "DCMT_CAP_CLOSED_LOOP_POSITION",
    "DCMT_CAP_CLOSED_LOOP_SPEED",
    "DCMT_CAP_CMD_WATCHDOG",
    "DCMT_CAP_LEVEL_1",
    "DCMT_CAP_LEVEL_2",
    "DCMT_CAP_LEVEL_3",
    "DCMT_CAP_OPEN_LOOP_CONTROL",
    "DCMT_CAP_PID_TUNING",
    "DCMT_MODE_CLOSED_POSITION",
    "DCMT_MODE_CLOSED_SPEED",
    "DCMT_MODE_OPEN_LOOP",
    "DCMT_MODULE_VER_MAJOR",
    "DCMT_MODULE_VER_MINOR",
    "DCMT_MODULE_VER_PATCH",
    "DCMT_OP_GET_STATE",
    "DCMT_OP_SET_BRAKE",
    "DCMT_OP_SET_MODE",
    "DCMT_OP_SET_OPEN_LOOP",
    "DCMT_OP_SET_PID",
    "DCMT_OP_SET_SETPOINT",
    "DCMT_STATE_FIXED_LEN",
    "DCMT_STATE_OFF_BRAKES",
    "DCMT_STATE_OFF_ESTOP",
    "DCMT_STATE_OFF_M1_PWM",
    "DCMT_STATE_OFF_M2_PWM",
    "DCMT_STATE_OFF_MODE",
    "DCMT_STATE_OFF_POS1",
    "DCMT_STATE_OFF_POS2",
    "DCMT_STATE_OFF_SP1",
    "DCMT_STATE_OFF_SP2",
    "DCMT_STATE_OFF_SPD1",
    "DCMT_STATE_OFF_SPD2",
    "DCMT_TYPE_ID",
    "DcmtStateResult",
    "dcmt_parse_state_payload",
    "dcmt_query_caps",
    "dcmt_query_state",
    "dcmt_query_version",
    "dcmt_query_watchdog",
    "dcmt_send_set_brake",
    "dcmt_send_set_mode",
    "dcmt_send_set_open_loop",
    "dcmt_send_set_pid",
    "dcmt_send_set_setpoint",
    "dcmt_send_set_watchdog",
]

DCMT_TYPE_ID = 0x02

DCMT_MODULE_VER_MAJOR = 1
DCMT_MODULE_VER_MINOR = 0
DCMT_MODULE_VER_PATCH = 0

DCMT_OP_SET_OPEN_LOOP = 0x01
DCMT_OP_SET_BRAKE = 0x02
DCMT_OP_SET_MODE = 0x03
DCMT_OP_SET_SETPOINT = 0x04
DCMT_OP_SET_PID = 0x05

DCMT_OP_GET_STATE = 0x80

#: GET_STATE payload (19 bytes):
#: ``[mode:u8][m1_pwm:i16][m2_pwm:i16][sp1:i16][sp2:i16]``
#: ``[pos1:i16][pos2:i16][spd1:i16][spd2:i16][brakes:u8][estop:u8]``.
#: ``sp1``/``sp2`` are ``BREAD_INVALID_I16`` in ``DCMT_MODE_OPEN_LOOP``;
#: ``spd1``/``spd2`` are ``BREAD_INVALID_I16`` unless ``DCMT_MODE_CLOSED_SPEED``.
DCMT_STATE_OFF_MODE = 0
DCMT_STATE_OFF_M1_PWM = 1
DCMT_STATE_OFF_M2_PWM = 3
DCMT_STATE_OFF_SP1 = 5
DCMT_STATE_OFF_SP2 = 7
DCMT_STATE_OFF_POS1 = 9
DCMT_STATE_OFF_POS2 = 11
DCMT_STATE_OFF_SPD1 = 13
DCMT_STATE_OFF_SPD2 = 15
DCMT_STATE_OFF_BRAKES = 17
DCMT_STATE_OFF_ESTOP = 18
DCMT_STATE_FIXED_LEN = 19

DCMT_MODE_OPEN_LOOP = 0x00
DCMT_MODE_CLOSED_POSITION = 0x01
DCMT_MODE_CLOSED_SPEED = 0x02

DCMT_CAP_LEVEL_1 = 0x01
DCMT_CAP_LEVEL_2 = 0x02
DCMT_CAP_LEVEL_3 = 0x03

DCMT_CAP_OPEN_LOOP_CONTROL = 1 << 0
DCMT_CAP_BRAKE_CONTROL = 1 << 1
DCMT_CAP_CLOSED_LOOP_POSITION = 1 << 2
DCMT_CAP_CLOSED_LOOP_SPEED = 1 << 3
DCMT_CAP_PID_TUNING = 1 << 4
DCMT_CAP_CMD_WATCHDOG = 1 << 5

DCMT_CAP_BASELINE_FLAGS = DCMT_CAP_OPEN_LOOP_CONTROL | DCMT_CAP_BRAKE_CONTROL

_STATE = struct.Struct("<BhhhhhhhhBB")
assert _STATE.size == DCMT_STATE_FIXED_LEN

_U8 = struct.Struct("<B")
_U8_U8 = struct.Struct("<BB")
_U16 = struct.Struct("<H")
_I16_I16 = struct.Struct("<hh")
_PID = struct.Struct("<BBBBBB")


@dataclass(frozen=True, slots=True)
class DcmtStateResult:
    """``dcmt_state_result_t``."""

    mode: int
    #: Current PWM output, always valid.
    m1_pwm: int
    m2_pwm: int
    #: Active setpoint; ``BREAD_INVALID_I16`` in OPEN_LOOP.
    sp1: int
    sp2: int
    #: Encoder position, always populated.
    pos1: int
    pos2: int
    #: Tachometer speed; ``BREAD_INVALID_I16`` unless CLOSED_SPEED.
    spd1: int
    spd2: int
    brakes: int
    estop: int


def dcmt_send_set_open_loop(m1_pwm: int, m2_pwm: int) -> bytes:
    """``DCMT_OP_SET_OPEN_LOOP`` payload: ``[m1_pwm:i16][m2_pwm:i16]``."""
    return _I16_I16.pack(i16("m1_pwm", m1_pwm), i16("m2_pwm", m2_pwm))


def dcmt_send_set_brake(m1_brake: int, m2_brake: int) -> bytes:
    """``DCMT_OP_SET_BRAKE`` payload: ``[m1_brake:u8][m2_brake:u8]``."""
    return _U8_U8.pack(u8("m1_brake", m1_brake), u8("m2_brake", m2_brake))


def dcmt_send_set_mode(mode: int) -> bytes:
    """``DCMT_OP_SET_MODE`` payload: ``[mode:u8]`` (``DCMT_MODE_*``)."""
    return _U8.pack(u8("mode", mode))


def dcmt_send_set_setpoint(target1: int, target2: int) -> bytes:
    """``DCMT_OP_SET_SETPOINT`` payload: ``[target1:i16][target2:i16]``."""
    return _I16_I16.pack(i16("target1", target1), i16("target2", target2))


def dcmt_send_set_pid(
    kp1_x10: int, ki1_x10: int, kd1_x10: int, kp2_x10: int, ki2_x10: int, kd2_x10: int
) -> bytes:
    """``DCMT_OP_SET_PID`` payload: six ``u8`` gains, each ten times the gain."""
    return _PID.pack(
        u8("kp1_x10", kp1_x10),
        u8("ki1_x10", ki1_x10),
        u8("kd1_x10", kd1_x10),
        u8("kp2_x10", kp2_x10),
        u8("ki2_x10", ki2_x10),
        u8("kd2_x10", kd2_x10),
    )


def dcmt_send_set_watchdog(timeout_ms: int) -> bytes:
    """``BREAD_OP_SET_WATCHDOG`` payload: ``[timeout_ms:u16]``, 0 = disarm."""
    return _U16.pack(u16("timeout_ms", timeout_ms))


def dcmt_query_state() -> bytes:
    """SET_REPLY payload requesting ``DCMT_OP_GET_STATE``."""
    return _U8.pack(DCMT_OP_GET_STATE)


def dcmt_query_version() -> bytes:
    """SET_REPLY payload requesting the version reply."""
    return _U8.pack(BREAD_OP_GET_VERSION)


def dcmt_query_caps() -> bytes:
    """SET_REPLY payload requesting ``BREAD_OP_GET_CAPS``."""
    return _U8.pack(BREAD_OP_GET_CAPS)


def dcmt_query_watchdog() -> bytes:
    """SET_REPLY payload requesting ``BREAD_OP_GET_WATCHDOG``."""
    return _U8.pack(BREAD_OP_GET_WATCHDOG)


def dcmt_parse_state_payload(payload: bytes) -> DcmtStateResult:
    """Parse a GET_STATE reply payload (``dcmt_parse_state_payload``).

    Like the C parser, this rejects any length other than
    ``DCMT_STATE_FIXED_LEN``, trailing bytes included.
    """
    require_exact_len("DCMT GET_STATE", payload, DCMT_STATE_FIXED_LEN)
    return DcmtStateResult(*_STATE.unpack_from(payload))
