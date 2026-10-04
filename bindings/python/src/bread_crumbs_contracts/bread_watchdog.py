"""Shared bus-liveness command watchdog (``bread_watchdog.h``).

Firmware boots disarmed; a controller arms it with SET_WATCHDOG. While
armed, any valid inbound frame refreshes it; expiry drives the slice's
actuators to their safe state without touching estop. Support is
advertised per type by a capability flag (``RLHT_CAP_CMD_WATCHDOG``,
``DCMT_CAP_CMD_WATCHDOG``); do not send SET_WATCHDOG unless it is set.
"""

import struct
from dataclasses import dataclass

from ._wire import require_len

__all__ = [
    "BREAD_OP_GET_WATCHDOG",
    "BREAD_OP_SET_WATCHDOG",
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

#: SET_WATCHDOG payload: ``[timeout_ms:u16]``, 0 = disarm.
BREAD_WATCHDOG_SET_PAYLOAD_LEN = 2

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
    #: Currently expired; clears on the next valid SET command.
    tripped: int
    #: Cumulative trips since boot, never cleared.
    trip_count: int


def bread_watchdog_parse_payload(payload: bytes) -> BreadWatchdogResult:
    """Parse a GET_WATCHDOG reply payload (``bread_watchdog_parse_payload``)."""
    require_len("GET_WATCHDOG", payload, BREAD_WATCHDOG_FIXED_LEN)
    armed, timeout_ms, tripped, trip_count = _WATCHDOG.unpack_from(payload)
    return BreadWatchdogResult(armed, timeout_ms, tripped, trip_count)
