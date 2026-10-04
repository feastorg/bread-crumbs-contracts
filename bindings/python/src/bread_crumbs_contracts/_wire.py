"""Argument range checks and payload length checks shared by the codec modules.

CRUMBS payload fields are little-endian; each module packs them with
``struct`` using explicit ``<`` format strings. The checks here turn an
out-of-range argument into a ``ValueError`` before ``struct`` could wrap or
reject it, and a short reply into a ``ValueError`` before ``struct`` could
raise its own error type.
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


def require_len(name: str, payload: bytes, fixed_len: int) -> None:
    """Reject a payload shorter than ``fixed_len``.

    Longer payloads are accepted: the contracts extend layouts by appending
    fields, so a newer slice may reply with trailing bytes an older parser
    does not know.
    """
    if len(payload) < fixed_len:
        raise ValueError(f"{name} payload needs {fixed_len} bytes, got {len(payload)}")
