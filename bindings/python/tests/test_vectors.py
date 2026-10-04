"""Every constant, encoder and parser against the C-generated golden vectors."""

import dataclasses
from typing import Any

import pytest

import bread_crumbs_contracts as bcc

from vectors import CONSTANTS, ENCODE, PARSE, encode_id, parse_id


@pytest.mark.parametrize(("name", "value"), sorted(CONSTANTS.items()))
def test_constant_matches_header(name: str, value: int) -> None:
    assert getattr(bcc, name) == value


@pytest.mark.parametrize("vec", ENCODE, ids=encode_id)
def test_encoder_matches_c_helper(vec: dict[str, Any]) -> None:
    encoder = getattr(bcc, vec["name"])
    assert encoder(**vec["args"]).hex() == vec["payload"]
    assert getattr(bcc, vec["type_id_name"]) == vec["type_id"]
    assert getattr(bcc, vec["opcode_name"]) == vec["opcode"]


@pytest.mark.parametrize("vec", PARSE, ids=parse_id)
def test_parser_matches_c_helper(vec: dict[str, Any]) -> None:
    """Python raises exactly where C returns nonzero and parses identically where it accepts."""
    parser = getattr(bcc, vec["name"])
    payload = bytes.fromhex(vec["payload"])
    if vec["rc"] == 0:
        assert dataclasses.asdict(parser(payload)) == vec["result"]
    else:
        assert vec["result"] is None
        with pytest.raises(ValueError):
            parser(payload)


def test_every_parser_has_short_and_over_long_vectors() -> None:
    """Each parser's length rule is recorded from C, not assumed.

    C rejects every short payload. Whether it rejects one trailing byte
    differs per parser today (#18 makes it uniform); the vectors say which.
    """
    rc_by_len: dict[str, dict[int, int]] = {}
    for vec in PARSE:
        rc_by_len.setdefault(vec["name"], {})[len(bytes.fromhex(vec["payload"]))] = vec["rc"]
    rejects_trailing_byte: set[str] = set()
    for name, rcs in rc_by_len.items():
        fixed = min(n for n, rc in rcs.items() if rc == 0)
        assert rcs.get(0) == -1, name
        assert rcs.get(fixed - 1) == -1, name
        assert fixed + 1 in rcs, f"{name} has no over-long vector"
        if rcs[fixed + 1] != 0:
            rejects_trailing_byte.add(name)
    assert rejects_trailing_byte == {"dcmt_parse_state_payload", "bread_watchdog_parse_payload"}


def test_every_payload_fits_a_frame() -> None:
    for vec in ENCODE + PARSE:
        assert len(bytes.fromhex(vec["payload"])) <= bcc.CRUMBS_MAX_PAYLOAD, vec["name"]


def test_every_python_encoder_and_parser_has_a_vector() -> None:
    """A codec function added without a vector is untested against C."""
    covered = {vec["name"] for vec in ENCODE} | {vec["name"] for vec in PARSE}
    codec_prefixes = ("rlht_send_", "rlht_query_", "dcmt_send_", "dcmt_query_")
    codec_suffixes = ("_parse_payload", "_parse_version")
    codec = {
        name
        for name in bcc.__all__
        if callable(getattr(bcc, name))
        and (name.startswith(codec_prefixes) or name.endswith(codec_suffixes))
    }
    assert "rlht_send_set_setpoints" in codec
    assert "bread_parse_version" in codec
    assert codec <= covered, sorted(codec - covered)
