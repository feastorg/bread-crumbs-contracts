"""Every ``#define *_OP_*`` in the headers must have a golden vector.

Adding an opcode to a header without a vector means the Python codec can
drift from it unnoticed, so this fails until the generator covers it.
"""

import re

import bread_crumbs_contracts as bcc

from vectors import ENCODE, HEADER_DIR, PARSE

_OP_DEFINE = re.compile(
    r"^#define\s+((RLHT|DCMT|BREAD)_OP_[A-Z0-9_]+)\s+(0x[0-9A-Fa-f]+|\d+)\b", re.M
)


def header_opcodes() -> dict[str, tuple[str, int]]:
    """``{macro: (family, value)}`` for every opcode define in include/bread."""
    found: dict[str, tuple[str, int]] = {}
    for header in sorted(HEADER_DIR.glob("*.h")):
        for match in _OP_DEFINE.finditer(header.read_text(encoding="utf-8")):
            macro, family, literal = match.groups()
            found[macro] = (family, int(literal, 0))
    return found


def covered_opcodes() -> dict[str, set[int]]:
    """``{family: opcodes}`` the vectors exercise, by type id of the frame.

    A send vector covers its opcode. A SET_REPLY vector covers the opcode
    named in its payload. A parse vector covers the reply's opcode.
    """
    family_of = {bcc.RLHT_TYPE_ID: "RLHT", bcc.DCMT_TYPE_ID: "DCMT"}
    covered: dict[str, set[int]] = {"RLHT": set(), "DCMT": set()}
    for vec in ENCODE:
        family = vec["name"].split("_", 1)[0].upper()
        if vec["opcode"] == bcc.CRUMBS_CMD_SET_REPLY:
            covered[family].add(bytes.fromhex(vec["payload"])[0])
        else:
            covered[family].add(vec["opcode"])
    for vec in PARSE:
        covered[family_of[vec["type_id"]]].add(vec["opcode"])
    return covered


def test_header_scan_finds_known_opcodes() -> None:
    found = header_opcodes()
    assert found["RLHT_OP_SET_SETPOINTS"] == ("RLHT", 0x02)
    assert found["DCMT_OP_GET_STATE"] == ("DCMT", 0x80)
    assert found["BREAD_OP_GET_CAPS"] == ("BREAD", 0x7F)


def test_every_header_opcode_has_a_vector() -> None:
    covered = covered_opcodes()
    missing: list[str] = []
    for macro, (family, value) in sorted(header_opcodes().items()):
        families = ("RLHT", "DCMT") if family == "BREAD" else (family,)
        if not all(value in covered[f] for f in families):
            missing.append(f"{macro}=0x{value:02X}")
    assert not missing, missing


def test_every_header_opcode_is_exported() -> None:
    for macro, (_family, value) in header_opcodes().items():
        assert getattr(bcc, macro) == value
