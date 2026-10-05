"""Shared bus-liveness command watchdog (``bread_watchdog.h``).

Firmware boots disarmed; a controller arms it with SET_WATCHDOG. While
armed, any valid inbound frame refreshes it; expiry drives the slice's
actuators to their safe state without touching estop. Support is
advertised per type by a capability flag (``RLHT_CAP_CMD_WATCHDOG``,
``DCMT_CAP_CMD_WATCHDOG``); do not send SET_WATCHDOG unless it is set.

What clears a trip depends on the firmware. Where the per-type
``*_CAP_CLEAR_WATCHDOG_TRIP`` flag is advertised the trip latches: only
CLEAR_WATCHDOG_TRIP, a deliberate local operator command on the slice
(firmware-defined, not any serial input), or a reboot clears it, and
SET_WATCHDOG re-arms without clearing. Without that flag, SET_WATCHDOG and
any other valid command frame (not a query) clear it, CLEAR_WATCHDOG_TRIP
included, so CLEAR_WATCHDOG_TRIP must not be sent there.
"""

import struct
from dataclasses import dataclass

from ._wire import require_exact_len

__all__ = [
    "BREAD_OP_CLEAR_WATCHDOG_TRIP",
    "BREAD_OP_GET_WATCHDOG",
    "BREAD_OP_SET_WATCHDOG",
    "BREAD_WATCHDOG_CLEAR_TRIP_PAYLOAD_LEN",
    "BREAD_WATCHDOG_FIXED_LEN",
    "BREAD_WATCHDOG_OFF_ARMED",
    "BREAD_WATCHDOG_OFF_TIMEOUT_MS",
    "BREAD_WATCHDOG_OFF_TRIPPED",
    "BREAD_WATCHDOG_OFF_TRIP_COUNT",
    "BREAD_WATCHDOG_SET_PAYLOAD_LEN",
    "BreadWatchdogResult",
    "bread_watchdog_parse_payload",
]

BREAD_OP_SET_WATCHDOG = 0x7E
BREAD_OP_GET_WATCHDOG = 0x7D
BREAD_OP_CLEAR_WATCHDOG_TRIP = 0x7C

#: SET_WATCHDOG payload: ``[timeout_ms:u16]``, 0 = disarm.
BREAD_WATCHDOG_SET_PAYLOAD_LEN = 2

#: CLEAR_WATCHDOG_TRIP payload: empty. Clears ``tripped`` only; it does not
#: arm, disarm or change the timeout, and does not reset ``trip_count``.
BREAD_WATCHDOG_CLEAR_TRIP_PAYLOAD_LEN = 0

#: GET_WATCHDOG payload: ``[armed:u8][timeout_ms:u16][tripped:u8][trip_count:u8]``.
BREAD_WATCHDOG_OFF_ARMED = 0
BREAD_WATCHDOG_OFF_TIMEOUT_MS = 1
BREAD_WATCHDOG_OFF_TRIPPED = 3
BREAD_WATCHDOG_OFF_TRIP_COUNT = 4
BREAD_WATCHDOG_FIXED_LEN = 5

_WATCHDOG = struct.Struct("<BHBB")
assert _WATCHDOG.size == BREAD_WATCHDOG_FIXED_LEN


@dataclass(frozen=True, slots=True)
class BreadWatchdogResult:
    """``bread_watchdog_result_t``."""

    armed: int
    #: Configured timeout; 0 when disarmed.
    timeout_ms: int
    #: Expired since the last clear. With ``*_CAP_CLEAR_WATCHDOG_TRIP`` it
    #: clears only on CLEAR_WATCHDOG_TRIP, a deliberate local operator
    #: command or reboot; without it, on the next valid command frame.
    tripped: int
    #: Cumulative trips since boot, never cleared.
    trip_count: int


def bread_watchdog_parse_payload(payload: bytes) -> BreadWatchdogResult:
    """Parse a GET_WATCHDOG reply payload (``bread_watchdog_parse_payload``).

    Like the C parser, this rejects any length other than
    ``BREAD_WATCHDOG_FIXED_LEN``, trailing bytes included.
    """
    require_exact_len("GET_WATCHDOG", payload, BREAD_WATCHDOG_FIXED_LEN)
    armed, timeout_ms, tripped, trip_count = _WATCHDOG.unpack_from(payload)
    return BreadWatchdogResult(armed, timeout_ms, tripped, trip_count)
