"""Argument range checks and payload length checks shared by the codec modules.

CRUMBS payload fields are little-endian; each module packs them with
``struct`` using explicit ``<`` format strings. The checks here turn an
out-of-range argument into a ``ValueError`` before ``struct`` could wrap or
reject it, and a reply of the wrong length into a ``ValueError`` before
``struct`` could raise its own error type.

The two length rules mirror the C parsers as they are today, not a single
policy: ``dcmt_parse_state_payload()`` and ``bread_watchdog_parse_payload()``
reject any length other than the fixed one, while
``rlht_parse_state_payload()``, ``bread_caps_parse_payload()`` and
``bread_parse_version()`` read a fixed prefix and ignore trailing bytes.
feastorg/bread-crumbs-contracts#18 is where the rule becomes uniform; each
Python parser follows its C namesake until then, and the golden vectors
record which lengths the C accepts.
"""

U8_MIN, U8_MAX = 0, 0xFF
U16_MIN, U16_MAX = 0, 0xFFFF
I16_MIN, I16_MAX = -0x8000, 0x7FFF


def _check_int(name: str, value: int, lo: int, hi: int) -> int:
    if not isinstance(value, int):  # pyright: ignore[reportUnnecessaryIsInstance]
        raise TypeError(f"{name} must be an int, got {type(value).__name__}")
    if not lo <= value <= hi:
        raise ValueError(f"{name} must be in {lo}..{hi}, got {value}")
    return value


def u8(name: str, value: int) -> int:
    """Return ``value`` if it fits a ``uint8_t``, else raise."""
    return _check_int(name, value, U8_MIN, U8_MAX)


def u16(name: str, value: int) -> int:
    """Return ``value`` if it fits a ``uint16_t``, else raise."""
    return _check_int(name, value, U16_MIN, U16_MAX)


def i16(name: str, value: int) -> int:
    """Return ``value`` if it fits an ``int16_t``, else raise."""
    return _check_int(name, value, I16_MIN, I16_MAX)


def require_exact_len(name: str, payload: bytes, fixed_len: int) -> None:
    """Reject a payload whose length is not exactly ``fixed_len``.

    The C check is ``data_len != FIXED_LEN``: a short payload and one with
    trailing bytes are both rejected.
    """
    if len(payload) != fixed_len:
        raise ValueError(f"{name} payload must be {fixed_len} bytes, got {len(payload)}")


def require_min_len(name: str, payload: bytes, fixed_len: int) -> None:
    """Reject a payload shorter than ``fixed_len``; trailing bytes are ignored.

    The C check is ``len < FIXED_LEN`` (or a field-by-field read that stops
    at the last fixed offset), so a longer payload parses as its prefix.
    """
    if len(payload) < fixed_len:
        raise ValueError(f"{name} payload needs {fixed_len} bytes, got {len(payload)}")
