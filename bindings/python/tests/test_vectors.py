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
    parser = getattr(bcc, vec["name"])
    payload = bytes.fromhex(vec["payload"])
    if vec["rc"] == 0:
        assert dataclasses.asdict(parser(payload)) == vec["result"]
    else:
        assert vec["result"] is None
        with pytest.raises(ValueError):
            parser(payload)


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
