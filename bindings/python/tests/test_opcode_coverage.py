"""Every named constant in the headers must be covered, by name.

An opcode added to a header without a golden vector, or any value define
added without a vectors.json constant and a Python export, is drift the
codec cannot see, so these fail until the generator and the package cover
it. The constants are the value defines plus what each
``CRUMBS_DEFINE_FAMILY`` declares: its ``PREFIX_TYPE_ID`` and every
``X(NAME, value)`` entry of its opcode list. Names are matched regardless
of how the value is written (``0x07``, ``(0x07)``, ``0x07u``, a cast, an
expression), so the value is never parsed here; vectors.json carries the
compiled value and test_vectors compares it with the Python constant.
"""

import re

import bread_crumbs_contracts as bcc

from vectors import CONSTANTS, ENCODE, HEADER_DIR, PARSE

# A value define: name, then blanks, then a value on the same line. A
# function-like macro has "(" right after its name and does not match; an
# include guard has nothing after its name and does not match either.
_VALUE_DEFINE = re.compile(r"^[ \t]*#[ \t]*define[ \t]+([A-Z][A-Z0-9_]*)[ \t]+\S", re.M)
# A family declaration and the opcode-list macro it names.
_FAMILY = re.compile(
    r"^[ \t]*CRUMBS_DEFINE_FAMILY\([ \t]*([A-Z][A-Z0-9_]*)[ \t]*,"
    r"[^,]*,[ \t]*([A-Z][A-Z0-9_]*)[ \t]*\)",
    re.M,
)
# An entry of an opcode list. Payload field lists are X(type, name) with a
# lowercase type, so they never match.
_OP_ENTRY = re.compile(r"\bX\([ \t]*([A-Z][A-Z0-9_]*)[ \t]*,")
_OP_NAME = re.compile(r"^(RLHT|DCMT|BREAD)_OP_")


def _macro_body(text: str, name: str) -> str:
    """The replacement text of function-like macro ``name(X)``, continuations joined."""
    match = re.search(rf"^[ \t]*#[ \t]*define[ \t]+{name}\(X\)((?:.*\\\n)*.*)$", text, re.M)
    assert match, f"{name}(X) is named by CRUMBS_DEFINE_FAMILY but not defined"
    return match.group(1)


def header_defines() -> dict[str, str]:
    """``{name: header file name}`` for every named constant in include/bread."""
    found: dict[str, str] = {}
    for header in sorted(HEADER_DIR.glob("*.h")):
        text = header.read_text(encoding="utf-8")
        for match in _VALUE_DEFINE.finditer(text):
            found[match.group(1)] = header.name
        for prefix, ops in _FAMILY.findall(text):
            found[f"{prefix}_TYPE_ID"] = header.name
            for name in _OP_ENTRY.findall(_macro_body(text, ops)):
                found[name] = header.name
    return found


def header_opcodes() -> dict[str, str]:
    """``{macro: family}`` for every ``*_OP_*`` define."""
    return {
        name: _OP_NAME.match(name).group(1)  # pyright: ignore[reportOptionalMemberAccess]
        for name in header_defines()
        if _OP_NAME.match(name)
    }


def covered_opcode_names() -> dict[str, set[str]]:
    """``{family: opcode macro names}`` the vectors exercise.

    A send vector covers its ``opcode_name`` for the family in its name. A
    parse vector covers the reply's ``opcode_name`` for the family of its
    ``type_id_name``. Query vectors carry ``CRUMBS_CMD_SET_REPLY`` and the
    requested opcode only as a byte, so the reply they request is covered by
    the parse vector of that reply instead.
    """
    covered: dict[str, set[str]] = {"RLHT": set(), "DCMT": set()}
    for vec in ENCODE:
        covered[vec["name"].split("_", 1)[0].upper()].add(vec["opcode_name"])
    for vec in PARSE:
        covered[vec["type_id_name"].split("_", 1)[0]].add(vec["opcode_name"])
    return covered


def test_header_scan_finds_known_defines_only() -> None:
    found = header_defines()
    assert found["RLHT_OP_SET_SETPOINTS"] == "rlht_ops.h"
    assert found["RLHT_OP_GET_STATE"] == "rlht_ops.h"
    assert found["DCMT_OP_SET_PID"] == "dcmt_ops.h"
    assert found["RLHT_TYPE_ID"] == "rlht_ops.h"
    assert found["DCMT_TYPE_ID"] == "dcmt_ops.h"
    assert found["DCMT_STATE_FIXED_LEN"] == "dcmt_ops.h"
    assert "RLHT_SET_PID_FIELDS" not in found, "payload field lists are not constants"
    assert "RLHT_OPS" not in found, "opcode lists are not constants"
    assert found["BREAD_MIN_CRUMBS_VERSION"] == "bread_version_helpers.h"
    assert "BREAD_OPS_H" not in found, "include guards are not value defines"
    assert "BREAD_IS_VALID_I16" not in found, "function-like macros are not value defines"
    assert set(header_opcodes()) >= {"RLHT_OP_SET_MODE", "DCMT_OP_GET_STATE", "BREAD_OP_GET_CAPS"}


def test_every_header_opcode_has_a_vector() -> None:
    covered = covered_opcode_names()
    missing = [
        name
        for name, family in sorted(header_opcodes().items())
        if not all(
            name in covered[f] for f in (("RLHT", "DCMT") if family == "BREAD" else (family,))
        )
    ]
    assert not missing, missing


def test_every_header_define_is_a_vector_constant() -> None:
    missing = sorted(set(header_defines()) - set(CONSTANTS))
    assert not missing, f"add to emit_constants() in gen_vectors.c: {missing}"


def test_every_header_define_is_exported() -> None:
    missing = sorted(set(header_defines()) - set(bcc.__all__))
    assert not missing, f"add to the Python package: {missing}"
