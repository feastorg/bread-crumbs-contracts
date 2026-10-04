"""Version reply parsing and compatibility policy (``bread_version_helpers.h``)."""

import struct
from dataclasses import dataclass

from ._wire import require_min_len

__all__ = [
    "BREAD_MIN_CRUMBS_VERSION",
    "BREAD_OP_GET_VERSION",
    "BREAD_VERSION_PAYLOAD_LEN",
    "BreadVersionResult",
    "bread_check_crumbs_compat",
    "bread_check_module_compat",
    "bread_parse_version",
]

#: Opcode of the version query. The headers have no macro for it;
#: ``rlht_query_version()`` and ``dcmt_query_version()`` request the literal
#: ``0x00``, and ``bread_parse_version()`` documents it as "opcode 0x00".
BREAD_OP_GET_VERSION = 0x00

#: Lowest peripheral CRUMBS version (major*10000 + minor*100 + patch) this
#: contract set accepts.
BREAD_MIN_CRUMBS_VERSION = 1200

#: Version payload: ``[CRUMBS_VERSION:u16][module_major:u8][module_minor:u8][module_patch:u8]``.
#: The headers have no macro for this length either; ``bread_parse_version()``
#: checks ``len < 5``.
BREAD_VERSION_PAYLOAD_LEN = 5

_VERSION = struct.Struct("<HBBB")
assert _VERSION.size == BREAD_VERSION_PAYLOAD_LEN


@dataclass(frozen=True, slots=True)
class BreadVersionResult:
    """The out-parameters of ``bread_parse_version``."""

    crumbs_ver: int
    mod_major: int
    mod_minor: int
    mod_patch: int


def bread_parse_version(payload: bytes) -> BreadVersionResult:
    """Parse a version reply payload (``bread_parse_version``).

    Like the C parser, this reads the 5-byte prefix and ignores any
    trailing bytes; a shorter payload is rejected.
    """
    require_min_len("version", payload, BREAD_VERSION_PAYLOAD_LEN)
    crumbs_ver, mod_major, mod_minor, mod_patch = _VERSION.unpack_from(payload)
    return BreadVersionResult(crumbs_ver, mod_major, mod_minor, mod_patch)


def bread_check_crumbs_compat(peripheral_ver: int) -> int:
    """``bread_check_crumbs_compat``: 0 when at or above the minimum, else -1."""
    return 0 if peripheral_ver >= BREAD_MIN_CRUMBS_VERSION else -1


def bread_check_module_compat(
    peri_major: int, peri_minor: int, expect_major: int, expect_minor: int
) -> int:
    """``bread_check_module_compat``.

    Returns 0 when compatible, -1 when MAJOR differs, -2 when the
    peripheral MINOR is below the expected MINOR. PATCH is ignored.
    """
    if peri_major != expect_major:
        return -1
    if peri_minor < expect_minor:
        return -2
    return 0
