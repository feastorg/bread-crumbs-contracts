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
from .bread_faults import BREAD_OP_GET_FAULTS
from .bread_version_helpers import BREAD_OP_GET_VERSION
from .bread_watchdog import BREAD_OP_GET_WATCHDOG

__all__ = [
    "DCMT_CAP_BASELINE_FLAGS",
    "DCMT_CAP_BRAKE_CONTROL",
    "DCMT_CAP_CLEAR_WATCHDOG_TRIP",
    "DCMT_CAP_CLOSED_LOOP_POSITION",
    "DCMT_CAP_CLOSED_LOOP_SPEED",
    "DCMT_CAP_CMD_WATCHDOG",
    "DCMT_CAP_ESTOP_HOLD",
    "DCMT_CAP_FAULTS",
    "DCMT_CAP_INPUTS",
    "DCMT_CAP_LEVEL_1",
    "DCMT_CAP_LEVEL_2",
    "DCMT_CAP_LEVEL_3",
    "DCMT_CAP_OPEN_LOOP_CONTROL",
    "DCMT_CAP_PID_TUNING",
    "DCMT_CAP_REFUSES",
    "DCMT_CAP_SETPOINT_LOOP",
    "DCMT_CAP_SPEED_FEEDFORWARD",
    "DCMT_CAP_WATCHDOG_LEASE",
    "DCMT_FAULT_M1_STALL",
    "DCMT_FAULT_M2_STALL",
    "DCMT_INPUTS_FIXED_LEN",
    "DCMT_INPUTS_OFF_BITS",
    "DCMT_INPUTS_OFF_P1A",
    "DCMT_INPUTS_OFF_P1B",
    "DCMT_INPUTS_OFF_P2A",
    "DCMT_INPUTS_OFF_P2B",
    "DCMT_INPUTS_OFF_VALID",
    "DCMT_INPUT_VALID_P1A",
    "DCMT_INPUT_VALID_P1B",
    "DCMT_INPUT_VALID_P2A",
    "DCMT_INPUT_VALID_P2B",
    "DCMT_MODE_CLOSED_POSITION",
    "DCMT_MODE_CLOSED_SPEED",
    "DCMT_MODE_OPEN_LOOP",
    "DCMT_MODULE_VER_MAJOR",
    "DCMT_MODULE_VER_MINOR",
    "DCMT_MODULE_VER_PATCH",
    "DCMT_OP_GET_INPUTS",
    "DCMT_OP_GET_STATE",
    "DCMT_OP_SET_BRAKE",
    "DCMT_OP_SET_MODE",
    "DCMT_OP_SET_OPEN_LOOP",
    "DCMT_OP_SET_PID",
    "DCMT_OP_SET_SETPOINT",
    "DCMT_OP_SET_SPEED_FF",
    "DCMT_PID_LOOP_POSITION",
    "DCMT_PID_LOOP_SPEED",
    "DCMT_PWM_MAX",
    "DCMT_SET_PID_PAYLOAD_LEN",
    "DCMT_SET_SETPOINT_LOOP_PAYLOAD_LEN",
    "DCMT_SET_SETPOINT_PAYLOAD_LEN",
    "DCMT_SET_SPEED_FF_OFF_RPM_PER_PWM1",
    "DCMT_SET_SPEED_FF_OFF_RPM_PER_PWM2",
    "DCMT_SET_SPEED_FF_OFF_STATIC1",
    "DCMT_SET_SPEED_FF_OFF_STATIC2",
    "DCMT_SET_SPEED_FF_OFF_TRIM1",
    "DCMT_SET_SPEED_FF_OFF_TRIM2",
    "DCMT_SET_SPEED_FF_PAYLOAD_LEN",
    "DCMT_SPEED_FF_TRIM_UNBOUNDED",
    "DCMT_STATE_FIXED_LEN",
    "DCMT_STATE_OFF_BRAKES",
    "DCMT_STATE_OFF_ESTOP",
    "DCMT_STATE_OFF_ESTOP_COUNT",
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
    "DcmtInputsResult",
    "DcmtStateResult",
    "dcmt_parse_inputs_payload",
    "dcmt_parse_state_payload",
    "dcmt_query_caps",
    "dcmt_query_faults",
    "dcmt_query_inputs",
    "dcmt_query_state",
    "dcmt_query_version",
    "dcmt_query_watchdog",
    "dcmt_send_clear_faults",
    "dcmt_send_clear_watchdog_trip",
    "dcmt_send_set_brake",
    "dcmt_send_set_mode",
    "dcmt_send_set_open_loop",
    "dcmt_send_set_pid",
    "dcmt_send_set_setpoint",
    "dcmt_send_set_setpoint_loop",
    "dcmt_send_set_speed_ff",
    "dcmt_send_set_watchdog",
    "dcmt_send_set_watchdog_lease",
]

DCMT_TYPE_ID = 0x02

#: Major 3 since contracts 0.13.0: SET_PID's ``DCMT_PID_LOOP_SPEED`` gains
#: keep their layout but are read in new units (the speed loop drives PWM
#: directly), a reinterpretation the versioning rule makes breaking. Major 2
#: since contracts 0.7.0: SET_PID changed layout. A controller built for one
#: major refuses firmware reporting another.
DCMT_MODULE_VER_MAJOR = 3
DCMT_MODULE_VER_MINOR = 0
DCMT_MODULE_VER_PATCH = 0

DCMT_OP_SET_OPEN_LOOP = 0x01
DCMT_OP_SET_BRAKE = 0x02
DCMT_OP_SET_MODE = 0x03
DCMT_OP_SET_SETPOINT = 0x04
DCMT_OP_SET_PID = 0x05
DCMT_OP_SET_SPEED_FF = 0x06

DCMT_OP_GET_STATE = 0x80
DCMT_OP_GET_INPUTS = 0x81

#: SET_PID payload (13 bytes):
#: ``[loop:u8][kp1:u16][ki1:u16][kd1:u16][kp2:u16][ki2:u16][kd2:u16]``, each
#: gain ``u16`` x1000 (0 to 65.535). ``loop`` is ``DCMT_PID_LOOP_*``. Firmware
#: rejects any other length and a loop value it does not define. Since
#: module major 3, ``DCMT_PID_LOOP_SPEED`` gains read kp as PWM per RPM of
#: speed error and ki as PWM per RPM-second; kd is accepted and ignored.
DCMT_SET_PID_PAYLOAD_LEN = 13
DCMT_PID_LOOP_POSITION = 0x01
DCMT_PID_LOOP_SPEED = 0x02

#: SET_SPEED_FF payload (8 bytes), behind ``DCMT_CAP_SPEED_FEEDFORWARD``:
#: ``[static1:u8][rpm_per_pwm1:u16][trim1:u8][static2:u8][rpm_per_pwm2:u16][trim2:u8]``.
#: Per motor, the speed loop's feedforward: ``static`` is PWM (0 to
#: ``DCMT_PWM_MAX``), ``rpm_per_pwm`` output RPM per PWM step above
#: ``static`` x1000 (0 means no speed term), ``trim`` the bound on the PI
#: trim in PWM (``DCMT_SPEED_FF_TRIM_UNBOUNDED`` means no bound). Firmware
#: rejects any other length.
DCMT_SET_SPEED_FF_OFF_STATIC1 = 0
DCMT_SET_SPEED_FF_OFF_RPM_PER_PWM1 = 1
DCMT_SET_SPEED_FF_OFF_TRIM1 = 3
DCMT_SET_SPEED_FF_OFF_STATIC2 = 4
DCMT_SET_SPEED_FF_OFF_RPM_PER_PWM2 = 5
DCMT_SET_SPEED_FF_OFF_TRIM2 = 7
DCMT_SET_SPEED_FF_PAYLOAD_LEN = 8
DCMT_SPEED_FF_TRIM_UNBOUNDED = 255

#: SET_SETPOINT: ``[target1:i16][target2:i16]``, or, on firmware advertising
#: ``DCMT_CAP_SETPOINT_LOOP``, ``[loop:u8][target1:i16][target2:i16]``. The
#: 5-byte form writes only the named loop's setpoints (``DCMT_PID_LOOP_*``);
#: with the capability the 4-byte form writes the position setpoints only,
#: and without it both. Send the 5-byte form only when it is advertised.
DCMT_SET_SETPOINT_PAYLOAD_LEN = 4
DCMT_SET_SETPOINT_LOOP_PAYLOAD_LEN = 5

#: GET_STATE payload (20 bytes):
#: ``[mode:u8][m1_pwm:i16][m2_pwm:i16][sp1:i16][sp2:i16]``
#: ``[pos1:i16][pos2:i16][spd1:i16][spd2:i16][brakes:u8][estop:u8][estop_count:u8]``.
#: ``estop_count`` counts e-stop presses since boot and wraps at 255; module
#: major 1 firmware sent the first 19 bytes only.
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
DCMT_STATE_OFF_ESTOP_COUNT = 19
DCMT_STATE_FIXED_LEN = 20

#: GET_INPUTS payload (10 bytes), behind ``DCMT_CAP_INPUTS``:
#: ``[valid:u8][bits:u8][p1a:u16][p1b:u16][p2a:u16][p2b:u16]``.
#: Raw averaged ADC counts of the two signal pins (a, b) of each encoder
#: header: port 1 is motor 1's (MCU A2/A3), port 2 motor 2's (A0/A1); the
#: terminal-to-pin mapping per board generation is in the Slice_DCMT
#: repository's docs/hardware-revisions.md. ``valid`` has one
#: ``DCMT_INPUT_VALID_*`` bit per pin, set when the pin is built as analog and
#: has a first average; a pin whose bit is clear reads 0. ``bits`` is the ADC
#: resolution: V = counts * Vref / 2**bits, Vref the board's 5 V supply.
DCMT_INPUTS_OFF_VALID = 0
DCMT_INPUTS_OFF_BITS = 1
DCMT_INPUTS_OFF_P1A = 2
DCMT_INPUTS_OFF_P1B = 4
DCMT_INPUTS_OFF_P2A = 6
DCMT_INPUTS_OFF_P2B = 8
DCMT_INPUTS_FIXED_LEN = 10

DCMT_INPUT_VALID_P1A = 0x01
DCMT_INPUT_VALID_P1B = 0x02
DCMT_INPUT_VALID_P2A = 0x04
DCMT_INPUT_VALID_P2B = 0x08

DCMT_MODE_OPEN_LOOP = 0x00
DCMT_MODE_CLOSED_POSITION = 0x01
DCMT_MODE_CLOSED_SPEED = 0x02

#: An open-loop PWM is ``-DCMT_PWM_MAX`` to ``DCMT_PWM_MAX``, inclusive.
#: Firmware advertising ``DCMT_CAP_REFUSES`` leaves a PWM outside it unchanged
#: and refuses with ``BREAD_REFUSE_UNDEFINED_VALUE``; older firmware clamps it.
#: The encoder checks only the wire type, so a value outside it still encodes.
DCMT_PWM_MAX = 255

DCMT_CAP_LEVEL_1 = 0x01
DCMT_CAP_LEVEL_2 = 0x02
DCMT_CAP_LEVEL_3 = 0x03

DCMT_CAP_OPEN_LOOP_CONTROL = 1 << 0
DCMT_CAP_BRAKE_CONTROL = 1 << 1
DCMT_CAP_CLOSED_LOOP_POSITION = 1 << 2
DCMT_CAP_CLOSED_LOOP_SPEED = 1 << 3
DCMT_CAP_PID_TUNING = 1 << 4
DCMT_CAP_CMD_WATCHDOG = 1 << 5
#: The watchdog trip latches until CLEAR_WATCHDOG_TRIP, a deliberate local
#: operator command or reboot; SET_WATCHDOG no longer clears it. Without
#: this flag, SET_WATCHDOG clears the trip.
DCMT_CAP_CLEAR_WATCHDOG_TRIP = 1 << 6
#: A hardware e-stop holds like a latched watchdog trip: brakes set,
#: position setpoints synced to the encoders until each brake is released or
#: a new setpoint arrives, motion and brake release refused while
#: e-stopped; a release moves nothing. Advertised only together with
#: ``DCMT_CAP_REFUSES``. Without this flag, a release resumes the stored mode
#: and setpoints.
DCMT_CAP_ESTOP_HOLD = 1 << 7
#: A SET the firmware does not act on in full is refused with a
#: ``BREAD_REFUSE_*`` reason, so GET_ACK's applied means every part was acted
#: on as sent; a frame acted on in part applies what it can and is refused.
#: Without this flag every frame reports applied, including ones the firmware
#: ignored.
DCMT_CAP_REFUSES = 1 << 8
#: SET_SETPOINT takes the 5-byte loop form, and the 4-byte form writes the
#: position setpoints only. Without it, the 4-byte form writes both.
DCMT_CAP_SETPOINT_LOOP = 1 << 9
#: SET_WATCHDOG takes the 3-byte lease form (``bread_watchdog``).
DCMT_CAP_WATCHDOG_LEASE = 1 << 10
#: Stall faults latch per motor: GET_FAULTS reports ``DCMT_FAULT_*`` bits and
#: CLEAR_FAULTS clears them; while any is latched both motors are braked and
#: motion commands are refused with ``BREAD_REFUSE_FAULT``.
DCMT_CAP_FAULTS = 1 << 11
#: GET_INPUTS reports the averaged ADC counts of the encoder-header pins
#: built as analog inputs. Without it, do not query GET_INPUTS.
DCMT_CAP_INPUTS = 1 << 12
#: SET_SPEED_FF sets the speed loop's per-motor feedforward. Advertised only
#: together with ``DCMT_CAP_CLOSED_LOOP_SPEED``. Without it, do not send
#: SET_SPEED_FF.
DCMT_CAP_SPEED_FEEDFORWARD = 1 << 13

#: GET_FAULTS bits: the motor was driven but its encoder did not advance.
DCMT_FAULT_M1_STALL = 0x01
DCMT_FAULT_M2_STALL = 0x02

DCMT_CAP_BASELINE_FLAGS = DCMT_CAP_OPEN_LOOP_CONTROL | DCMT_CAP_BRAKE_CONTROL

_STATE = struct.Struct("<BhhhhhhhhBBB")
assert _STATE.size == DCMT_STATE_FIXED_LEN
_INPUTS = struct.Struct("<BBHHHH")
assert _INPUTS.size == DCMT_INPUTS_FIXED_LEN

_U8 = struct.Struct("<B")
_U8_U8 = struct.Struct("<BB")
_U16 = struct.Struct("<H")
_I16_I16 = struct.Struct("<hh")
_PID = struct.Struct("<BHHHHHH")
assert _PID.size == DCMT_SET_PID_PAYLOAD_LEN
_SPEED_FF = struct.Struct("<BHBBHB")
assert _SPEED_FF.size == DCMT_SET_SPEED_FF_PAYLOAD_LEN
_U16_U8 = struct.Struct("<HB")
_SETPOINT_LOOP = struct.Struct("<Bhh")
assert _SETPOINT_LOOP.size == DCMT_SET_SETPOINT_LOOP_PAYLOAD_LEN


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
    #: E-stop presses since boot, wrapping at 255.
    estop_count: int


@dataclass(frozen=True, slots=True)
class DcmtInputsResult:
    """``dcmt_inputs_result_t``."""

    #: ``DCMT_INPUT_VALID_*`` bits: the pin is built as analog and has a first average.
    valid: int
    #: ADC resolution in bits.
    bits: int
    #: Averaged ADC counts; 0 where the pin's valid bit is clear.
    p1a: int
    p1b: int
    p2a: int
    p2b: int


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


def dcmt_send_set_setpoint_loop(loop: int, target1: int, target2: int) -> bytes:
    """``DCMT_OP_SET_SETPOINT`` 5-byte payload: ``[loop:u8][target1:i16][target2:i16]``.

    Writes only the named loop's setpoints. Send only when
    ``DCMT_CAP_SETPOINT_LOOP`` is advertised.
    """
    return _SETPOINT_LOOP.pack(u8("loop", loop), i16("target1", target1), i16("target2", target2))


def dcmt_send_set_pid(
    loop: int,
    kp1_x1000: int,
    ki1_x1000: int,
    kd1_x1000: int,
    kp2_x1000: int,
    ki2_x1000: int,
    kd2_x1000: int,
) -> bytes:
    """``DCMT_OP_SET_PID`` payload: ``loop`` (``DCMT_PID_LOOP_*``), then six
    ``u16`` gains, each a thousand times the gain.

    Send ``DCMT_PID_LOOP_SPEED`` only when ``DCMT_CAP_CLOSED_LOOP_SPEED`` is
    advertised.
    """
    return _PID.pack(
        u8("loop", loop),
        u16("kp1_x1000", kp1_x1000),
        u16("ki1_x1000", ki1_x1000),
        u16("kd1_x1000", kd1_x1000),
        u16("kp2_x1000", kp2_x1000),
        u16("ki2_x1000", ki2_x1000),
        u16("kd2_x1000", kd2_x1000),
    )


def dcmt_send_set_speed_ff(
    static1: int,
    rpm_per_pwm1_x1000: int,
    trim1: int,
    static2: int,
    rpm_per_pwm2_x1000: int,
    trim2: int,
) -> bytes:
    """``DCMT_OP_SET_SPEED_FF`` payload: per motor ``static`` (PWM), then
    ``rpm_per_pwm`` (RPM per PWM step, a thousand times the value), then
    ``trim`` (PWM; ``DCMT_SPEED_FF_TRIM_UNBOUNDED`` for no bound).

    Send only when ``DCMT_CAP_SPEED_FEEDFORWARD`` is advertised.
    """
    return _SPEED_FF.pack(
        u8("static1", static1),
        u16("rpm_per_pwm1_x1000", rpm_per_pwm1_x1000),
        u8("trim1", trim1),
        u8("static2", static2),
        u16("rpm_per_pwm2_x1000", rpm_per_pwm2_x1000),
        u8("trim2", trim2),
    )


def dcmt_send_set_watchdog(timeout_ms: int) -> bytes:
    """``BREAD_OP_SET_WATCHDOG`` payload: ``[timeout_ms:u16]``, 0 = disarm."""
    return _U16.pack(u16("timeout_ms", timeout_ms))


def dcmt_send_set_watchdog_lease(timeout_ms: int, flags: int) -> bytes:
    """``BREAD_OP_SET_WATCHDOG`` lease payload: ``[timeout_ms:u16][flags:u8]``.

    Send only when ``DCMT_CAP_WATCHDOG_LEASE`` is advertised.
    """
    return _U16_U8.pack(u16("timeout_ms", timeout_ms), u8("flags", flags))


def dcmt_send_clear_faults() -> bytes:
    """``BREAD_OP_CLEAR_FAULTS`` payload: empty.

    Send only when ``DCMT_CAP_FAULTS`` is advertised, and only on an
    operator's request.
    """
    return b""


def dcmt_send_clear_watchdog_trip() -> bytes:
    """``BREAD_OP_CLEAR_WATCHDOG_TRIP`` payload: empty.

    Send only when ``DCMT_CAP_CLEAR_WATCHDOG_TRIP`` is advertised, and only
    on an operator's request.
    """
    return b""


def dcmt_query_state() -> bytes:
    """SET_REPLY payload requesting ``DCMT_OP_GET_STATE``."""
    return _U8.pack(DCMT_OP_GET_STATE)


def dcmt_query_inputs() -> bytes:
    """SET_REPLY payload requesting ``DCMT_OP_GET_INPUTS``.

    Send only when ``DCMT_CAP_INPUTS`` is advertised.
    """
    return _U8.pack(DCMT_OP_GET_INPUTS)


def dcmt_query_version() -> bytes:
    """SET_REPLY payload requesting the version reply."""
    return _U8.pack(BREAD_OP_GET_VERSION)


def dcmt_query_caps() -> bytes:
    """SET_REPLY payload requesting ``BREAD_OP_GET_CAPS``."""
    return _U8.pack(BREAD_OP_GET_CAPS)


def dcmt_query_faults() -> bytes:
    """SET_REPLY payload requesting ``BREAD_OP_GET_FAULTS``."""
    return _U8.pack(BREAD_OP_GET_FAULTS)


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


def dcmt_parse_inputs_payload(payload: bytes) -> DcmtInputsResult:
    """Parse a GET_INPUTS reply payload (``dcmt_parse_inputs_payload``).

    Like the C parser, this rejects any length other than
    ``DCMT_INPUTS_FIXED_LEN``, trailing bytes included.
    """
    require_exact_len("DCMT GET_INPUTS", payload, DCMT_INPUTS_FIXED_LEN)
    return DcmtInputsResult(*_INPUTS.unpack_from(payload))
