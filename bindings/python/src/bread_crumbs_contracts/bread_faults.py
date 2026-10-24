"""Latched faults, shared across BREADS-compatible Slices (``bread_faults.h``).

A fault is a condition the firmware detects on one channel (a stalled motor,
an over-temperature heater) and latches until CLEAR_FAULTS, a deliberate local
operator command on the slice, or a reboot clears it. While any fault is
latched the slice holds its safe state on every channel and refuses a command
that could start motion or heat with ``BREAD_REFUSE_FAULT``. Clearing resumes
nothing. The bits are per family (``DCMT_FAULT_*``, ``RLHT_FAULT_*``); support
is advertised per type (``DCMT_CAP_FAULTS``, ``RLHT_CAP_FAULTS``), and a
controller must not send CLEAR_FAULTS or query GET_FAULTS without it.
"""

import struct
from dataclasses import dataclass

from ._wire import require_min_len

__all__ = [
    "BREAD_FAULTS_CLEAR_PAYLOAD_LEN",
    "BREAD_FAULTS_FIXED_LEN",
    "BREAD_FAULTS_OFF_FAULTS",
    "BREAD_OP_CLEAR_FAULTS",
    "BREAD_OP_GET_FAULTS",
    "BreadFaultsResult",
    "bread_faults_parse_payload",
]

BREAD_OP_CLEAR_FAULTS = 0x7B
BREAD_OP_GET_FAULTS = 0x7A

#: CLEAR_FAULTS payload: empty. Clears every latched fault and nothing else;
#: an operator's acknowledgement, never sent on a controller's own initiative.
BREAD_FAULTS_CLEAR_PAYLOAD_LEN = 0

#: GET_FAULTS payload: ``[faults:u8]``, one bit per latched fault, per family.
BREAD_FAULTS_OFF_FAULTS = 0
BREAD_FAULTS_FIXED_LEN = 1

_FAULTS = struct.Struct("<B")
assert _FAULTS.size == BREAD_FAULTS_FIXED_LEN


@dataclass(frozen=True, slots=True)
class BreadFaultsResult:
    """``bread_faults_result_t``."""

    #: One bit per latched fault (``DCMT_FAULT_*`` or ``RLHT_FAULT_*``).
    faults: int


def bread_faults_parse_payload(payload: bytes) -> BreadFaultsResult:
    """Parse a GET_FAULTS reply payload (``bread_faults_parse_payload``).

    Like the C parser, this reads the fixed prefix and ignores trailing bytes.
    """
    require_min_len("GET_FAULTS", payload, BREAD_FAULTS_FIXED_LEN)
    (faults,) = _FAULTS.unpack_from(payload)
    return BreadFaultsResult(faults)
