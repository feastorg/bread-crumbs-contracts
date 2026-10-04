"""Shared sentinels and the capability query (``bread_caps.h``)."""

import struct
from dataclasses import dataclass

from ._wire import require_min_len

__all__ = [
    "BREAD_CAPS_SCHEMA_V1",
    "BREAD_CAPS_V1_PAYLOAD_LEN",
    "BREAD_INVALID_I16",
    "BREAD_INVALID_U8",
    "BREAD_OP_GET_CAPS",
    "BreadCapsResult",
    "bread_caps_parse_payload",
    "bread_is_valid_i16",
    "bread_is_valid_u8",
]

#: Fixed-layout fields carry these instead of zero when the field does not
#: apply in the current mode or build. Check with the ``bread_is_valid_*``
#: helpers before consuming such a field.
BREAD_INVALID_I16 = -32768
BREAD_INVALID_U8 = 0xFF

#: Capability query opcode shared by every BREADS-compatible slice.
BREAD_OP_GET_CAPS = 0x7F

#: Capability payload schema version.
BREAD_CAPS_SCHEMA_V1 = 0x01

#: v1 payload: ``[schema:u8][level:u8][flags:u32]``.
BREAD_CAPS_V1_PAYLOAD_LEN = 6

_CAPS_V1 = struct.Struct("<BBI")
assert _CAPS_V1.size == BREAD_CAPS_V1_PAYLOAD_LEN


def bread_is_valid_i16(value: int) -> bool:
    """``BREAD_IS_VALID_I16``: the field is populated, not the sentinel."""
    return value != BREAD_INVALID_I16


def bread_is_valid_u8(value: int) -> bool:
    """``BREAD_IS_VALID_U8``: the field is populated, not the sentinel."""
    return value != BREAD_INVALID_U8


@dataclass(frozen=True, slots=True)
class BreadCapsResult:
    """``bread_caps_result_t``."""

    schema: int
    level: int
    flags: int


def bread_caps_parse_payload(payload: bytes) -> BreadCapsResult:
    """Parse a GET_CAPS reply payload (``bread_caps_parse_payload``).

    Like the C parser, this reads the 6-byte v1 prefix and ignores any
    trailing bytes; a shorter payload is rejected.
    """
    require_min_len("GET_CAPS", payload, BREAD_CAPS_V1_PAYLOAD_LEN)
    schema, level, flags = _CAPS_V1.unpack_from(payload)
    return BreadCapsResult(schema, level, flags)
