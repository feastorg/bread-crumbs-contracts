"""The few CRUMBS protocol constants the contracts depend on (``crumbs.h``).

Framing, CRC and I2C stay in the transport (CRUMBS itself, or a Python
binding of it). This package stops at the message level: a type id, an
opcode and the payload bytes between the frame header and the CRC.
"""

__all__ = [
    "CRUMBS_CMD_SET_REPLY",
    "CRUMBS_MAX_PAYLOAD",
    "CRUMBS_TYPE_ID_ANY",
]

#: Wildcard type id; a peripheral never rejects a frame carrying it.
CRUMBS_TYPE_ID_ANY = 0x00

#: Opcode of the SET_REPLY frame a controller sends before reading a reply.
#: Its one-byte payload names the opcode whose reply the peripheral should
#: build next. The ``*_query_*`` encoders return that payload.
CRUMBS_CMD_SET_REPLY = 0xFE

#: Largest payload a CRUMBS frame carries.
CRUMBS_MAX_PAYLOAD = 27
