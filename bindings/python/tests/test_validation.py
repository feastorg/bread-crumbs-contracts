"""Argument range checks, short and extended payloads, and the compat helpers."""

import dataclasses
from collections.abc import Callable
from typing import Any

import pytest

import bread_crumbs_contracts as bcc

from vectors import PARSE

U8 = (0, 0xFF)
U16 = (0, 0xFFFF)
I16 = (-0x8000, 0x7FFF)

PID_OK = {
    "kp1_x10": 0, "ki1_x10": 0, "kd1_x10": 0, "kp2_x10": 0, "ki2_x10": 0, "kd2_x10": 0
}  # fmt: skip

# (encoder, in-range kwargs, field under test, bounds)
RANGE_CASES: list[tuple[Callable[..., bytes], dict[str, int], str, tuple[int, int]]] = [
    (bcc.rlht_send_set_mode, {"mode": 0}, "mode", U8),
    (bcc.rlht_send_set_setpoints, {"sp1_deci_c": 0, "sp2_deci_c": 0}, "sp1_deci_c", I16),
    (bcc.rlht_send_set_setpoints, {"sp1_deci_c": 0, "sp2_deci_c": 0}, "sp2_deci_c", I16),
    *[(bcc.rlht_send_set_pid_x10, PID_OK, field, U8) for field in PID_OK],
    (bcc.rlht_send_set_periods, {"p1_ms": 0, "p2_ms": 0}, "p1_ms", U16),
    (bcc.rlht_send_set_periods, {"p1_ms": 0, "p2_ms": 0}, "p2_ms", U16),
    (bcc.rlht_send_set_tc_select, {"tc1": 0, "tc2": 0}, "tc1", U8),
    (bcc.rlht_send_set_tc_select, {"tc1": 0, "tc2": 0}, "tc2", U8),
    (bcc.rlht_send_set_open_duty, {"duty1_pct": 0, "duty2_pct": 0}, "duty1_pct", U8),
    (bcc.rlht_send_set_open_duty, {"duty1_pct": 0, "duty2_pct": 0}, "duty2_pct", U8),
    (bcc.rlht_send_set_watchdog, {"timeout_ms": 0}, "timeout_ms", U16),
    (bcc.dcmt_send_set_open_loop, {"m1_pwm": 0, "m2_pwm": 0}, "m1_pwm", I16),
    (bcc.dcmt_send_set_open_loop, {"m1_pwm": 0, "m2_pwm": 0}, "m2_pwm", I16),
    (bcc.dcmt_send_set_brake, {"m1_brake": 0, "m2_brake": 0}, "m1_brake", U8),
    (bcc.dcmt_send_set_brake, {"m1_brake": 0, "m2_brake": 0}, "m2_brake", U8),
    (bcc.dcmt_send_set_mode, {"mode": 0}, "mode", U8),
    (bcc.dcmt_send_set_setpoint, {"target1": 0, "target2": 0}, "target1", I16),
    (bcc.dcmt_send_set_setpoint, {"target1": 0, "target2": 0}, "target2", I16),
    *[(bcc.dcmt_send_set_pid, PID_OK, field, U8) for field in PID_OK],
    (bcc.dcmt_send_set_watchdog, {"timeout_ms": 0}, "timeout_ms", U16),
]


def _range_id(case: tuple[Callable[..., bytes], dict[str, int], str, tuple[int, int]]) -> str:
    return f"{case[0].__name__}.{case[2]}"


@pytest.mark.parametrize("case", RANGE_CASES, ids=_range_id)
def test_encoder_rejects_out_of_range(
    case: tuple[Callable[..., bytes], dict[str, int], str, tuple[int, int]],
) -> None:
    encoder, kwargs, field, (lo, hi) = case
    for bad in (lo - 1, hi + 1):
        with pytest.raises(ValueError, match=field):
            encoder(**{**kwargs, field: bad})
    assert encoder(**{**kwargs, field: lo})
    assert encoder(**{**kwargs, field: hi})


@pytest.mark.parametrize("case", RANGE_CASES, ids=_range_id)
def test_encoder_rejects_non_int(
    case: tuple[Callable[..., bytes], dict[str, int], str, tuple[int, int]],
) -> None:
    encoder, kwargs, field, _bounds = case
    with pytest.raises(TypeError, match=field):
        encoder(**{**kwargs, field: 1.5})


def test_every_encoder_field_has_a_range_case() -> None:
    """Each keyword of each ``*_send_*`` encoder appears in RANGE_CASES."""
    expected = {
        (name, field)
        for name in bcc.__all__
        if "_send_" in name
        for field in getattr(bcc, name).__code__.co_varnames[
            : getattr(bcc, name).__code__.co_argcount
        ]
    }
    assert ("rlht_send_set_setpoints", "sp2_deci_c") in expected
    assert expected == {(case[0].__name__, case[2]) for case in RANGE_CASES}


# Each parser applies the length rule its C namesake applies today (#18 is
# where the rule becomes uniform): exact length, or a fixed prefix with
# trailing bytes ignored.
PARSER_FIXED_LEN: dict[str, int] = {
    "rlht_parse_state_payload": bcc.RLHT_STATE_FIXED_LEN,
    "dcmt_parse_state_payload": bcc.DCMT_STATE_FIXED_LEN,
    "bread_caps_parse_payload": bcc.BREAD_CAPS_V1_PAYLOAD_LEN,
    "bread_watchdog_parse_payload": bcc.BREAD_WATCHDOG_FIXED_LEN,
    "bread_parse_version": bcc.BREAD_VERSION_PAYLOAD_LEN,
}
EXACT_LENGTH_PARSERS = {"dcmt_parse_state_payload", "bread_watchdog_parse_payload"}

_FIXED_LEN_PARSE: list[dict[str, Any]] = [
    vec
    for vec in PARSE
    if vec["rc"] == 0 and len(bytes.fromhex(vec["payload"])) == PARSER_FIXED_LEN[vec["name"]]
]


def test_every_parser_has_a_fixed_length_vector() -> None:
    assert {vec["name"] for vec in _FIXED_LEN_PARSE} == set(PARSER_FIXED_LEN)


@pytest.mark.parametrize("vec", _FIXED_LEN_PARSE, ids=lambda v: v["name"])
def test_parser_length_rule_mirrors_c(vec: dict[str, Any]) -> None:
    parser = getattr(bcc, vec["name"])
    payload = bytes.fromhex(vec["payload"])
    if vec["name"] in EXACT_LENGTH_PARSERS:
        with pytest.raises(ValueError):
            parser(payload + b"\xff")
    else:
        assert parser(payload + b"\xff\x00") == parser(payload)


@pytest.mark.parametrize("vec", _FIXED_LEN_PARSE, ids=lambda v: v["name"])
def test_parser_rejects_every_shorter_length(vec: dict[str, Any]) -> None:
    parser = getattr(bcc, vec["name"])
    payload = bytes.fromhex(vec["payload"])
    for n in range(len(payload)):
        with pytest.raises(ValueError):
            parser(payload[:n])


def test_results_are_frozen() -> None:
    state = bcc.dcmt_parse_state_payload(bytes(bcc.DCMT_STATE_FIXED_LEN))
    with pytest.raises(dataclasses.FrozenInstanceError):
        state.mode = 1  # pyright: ignore[reportAttributeAccessIssue]


def test_sentinel_helpers() -> None:
    assert not bcc.bread_is_valid_i16(bcc.BREAD_INVALID_I16)
    assert bcc.bread_is_valid_i16(0)
    assert bcc.bread_is_valid_i16(-32767)
    assert not bcc.bread_is_valid_u8(bcc.BREAD_INVALID_U8)
    assert bcc.bread_is_valid_u8(0)


def test_crumbs_compat() -> None:
    assert bcc.bread_check_crumbs_compat(bcc.BREAD_MIN_CRUMBS_VERSION) == 0
    assert bcc.bread_check_crumbs_compat(1400) == 0
    assert bcc.bread_check_crumbs_compat(bcc.BREAD_MIN_CRUMBS_VERSION - 1) == -1


def test_module_compat() -> None:
    assert bcc.bread_check_module_compat(1, 2, 1, 1) == 0
    assert bcc.bread_check_module_compat(1, 1, 1, 1) == 0
    assert bcc.bread_check_module_compat(2, 0, 1, 0) == -1
    assert bcc.bread_check_module_compat(1, 0, 1, 1) == -2
