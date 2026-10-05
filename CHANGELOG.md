# Changelog

## [Unreleased]

## [0.6.0] - 2026-10-05

### Changed

- `rlht_ops.h` and `dcmt_ops.h` declare their type and opcodes once with
  CRUMBS' `CRUMBS_DEFINE_FAMILY` and every payload layout once with
  `CRUMBS_DEFINE_PAYLOAD` (#18). The build now fails on a duplicate
  opcode or a field list that does not sum to its stated size. The bytes
  on the wire are unchanged for every opcode: each `*_send_*` and
  `*_query_*` helper writes the same frame as 0.5.0 and each state parser
  returns the same result for the same payload, and
  `tests/golden_vectors/vectors.json` regenerates without a difference.
- The `*_send_*` wrappers keep their scalar parameters and pack through
  the generated payload structs; their signatures are unchanged.
- `RLHT_TYPE_ID`, `DCMT_TYPE_ID` and the `RLHT_OP_*` / `DCMT_OP_*` opcodes
  are enum constants instead of macros, with the same values. They no
  longer work in `#if`. In C++ each family's constants have their own
  unnamed enum type, so mixing two families now warns where the macros
  did not: `r ? RLHT_TYPE_ID : DCMT_TYPE_ID` and comparing an RLHT
  opcode with a DCMT one raise `-Wenum-compare` (on by default in g++),
  and adding them raises `-Wdeprecated-enum-enum-conversion` in C++20.
- The headers stop with `#error` on CRUMBS older than 0.14.0, whose
  `crumbs_ops.h` lacks the family and payload macros.
- `dcmt_state_result_t` is a typedef of the generated `dcmt_state_t`, with
  the same members in the same order. `DCMT_STATE_OFF_*` and
  `DCMT_STATE_FIXED_LEN` stay, and a test holds them to the field list.
- `rlht_parse_state_payload()` leaves `out` untouched when it rejects a
  payload; it used to fill the fields before the first one that did not
  fit. Return codes and the length rules are unchanged:
  `rlht_parse_state_payload()` ignores trailing bytes and
  `dcmt_parse_state_payload()` requires exactly 19.
- The Python opcode-coverage test reads the opcode list and type of each
  `CRUMBS_DEFINE_FAMILY` as well as `#define`s, so an opcode added to a
  list without a golden vector still fails it.
- Requires CRUMBS 0.14.0 or newer for `crumbs_ops.h` (`library.json`
  `^0.14.0`, `find_package(crumbs 0.14)`), and so C11 or C++11.

### Added

- `BREAD_OP_CLEAR_WATCHDOG_TRIP` (0x7C, empty payload,
  `BREAD_WATCHDOG_CLEAR_TRIP_PAYLOAD_LEN` 0) in `bread_watchdog.h`, so
  re-arming the watchdog and clearing a trip are separate operations
  (feastorg/Slice_DCMT#26). It clears `tripped` and nothing else: it does
  not arm, disarm or change the timeout, and `trip_count` is kept. Sent by
  `dcmt_send_clear_watchdog_trip()` / `rlht_send_clear_watchdog_trip()`.
- Capability flags `DCMT_CAP_CLEAR_WATCHDOG_TRIP` (bit 6) and
  `RLHT_CAP_CLEAR_WATCHDOG_TRIP` (bit 7). Firmware that advertises one
  latches the trip until `BREAD_OP_CLEAR_WATCHDOG_TRIP`, a deliberate
  local operator command on the slice (firmware-defined, not any serial
  input), or a reboot; `BREAD_OP_SET_WATCHDOG` and ordinary frames no
  longer clear it there, and a non-empty CLEAR payload is rejected.
  Firmware without the flag behaves as before: `BREAD_OP_SET_WATCHDOG`
  and any other valid command frame clear the trip, so
  `BREAD_OP_CLEAR_WATCHDOG_TRIP` must not be sent to it. The `tripped`
  documentation in `bread_watchdog.h` says so. Opcode and payload bytes
  are unchanged; `tests/golden_vectors/vectors.json` gains the new
  constants and one send vector per family, and every existing record
  is unchanged.
- **Not purely additive for controllers:** on firmware that advertises
  the new flag, `BREAD_OP_SET_WATCHDOG` no longer clears a trip, so a
  controller that predates the flag (including released
  anolis-provider-bread) cannot clear a trip remotely; only a reboot or
  a local operator command does. Ship the controller release that
  understands the flag with or before such firmware. The capabilities
  Extension Policy records this exception.
- The Python codec exports the new opcode, payload length, flags and
  `dcmt_send_clear_watchdog_trip()` / `rlht_send_clear_watchdog_trip()`.
- Generated payload structs and codecs for Slice firmware to unpack SETs
  and pack replies with: `rlht_set_mode`, `rlht_set_setpoints`,
  `rlht_set_pid`, `rlht_set_periods`, `rlht_set_tc_select`,
  `rlht_set_open_duty`, `rlht_state`, `dcmt_set_open_loop`,
  `dcmt_set_brake`, `dcmt_set_mode`, `dcmt_set_setpoint`, `dcmt_set_pid`
  and `dcmt_state`, each with `_t`, `_wire_size`, `_pack()` and
  `_unpack()`.
- `tests/payload_roundtrip/`: one round-trip test per family. Each SET goes
  through its wrapper onto a fake bus, is checked against the bytes its
  layout comment states and unpacked with the Slice-side `_unpack()`; each
  GET_STATE reply is packed with the Slice-side `_pack()` and read back
  through the getter.
- The compile smoke test also builds as C++11 with `-Wall -Wextra -Werror`
  when a C++ compiler is available.

## [0.5.0] - 2026-10-03

### Added

- Added a pure-Python codec for the contracts (#20) in `bindings/python`,
  distributed as `bread-crumbs-contracts` (import `bread_crumbs_contracts`),
  Python 3.11+, no runtime dependencies. One module per header, named after
  the C helpers: an encoder per `*_send_*` and `*_query_*` helper returning
  the payload bytes, a parser per reply (`rlht_parse_state_payload()`,
  `dcmt_parse_state_payload()`, `bread_caps_parse_payload()`,
  `bread_watchdog_parse_payload()`, `bread_parse_version()`) returning a
  frozen dataclass, and every `#define` constant. Encoders raise
  `ValueError` on out-of-range arguments; each parser applies the length
  rule of its C namesake (`dcmt_parse_state_payload()` and
  `bread_watchdog_parse_payload()` require the exact length, the others
  read a fixed prefix and ignore trailing bytes) until #18 makes the rule
  uniform.
- Added `tests/golden_vectors/gen_vectors.c`, built with the test targets as
  `bread_contracts_gen_vectors`, which calls every C send helper through a
  capturing write function and every C parser on known replies and writes
  the tracked `tests/golden_vectors/vectors.json`. The Python tests check
  every constant, encoder and parser against it, and fail on a header
  opcode or a Python codec function with no vector.
- CI gained a `golden-vectors` job that regenerates `vectors.json` and
  fails on any difference, and a `python` job that runs ruff, pyright and
  pytest through uv.
- Added `bread_crumbs_contracts.__version__`, read from the installed
  distribution's metadata, and a test that holds it equal to the version in
  `library.json`, `library.properties` and `CMakeLists.txt`, so a bump that
  misses one of them fails CI.
- The release workflow now publishes the Python package (#21). On a `v*`
  tag it checks that the tag names the version in `library.json`,
  `library.properties`, `pyproject.toml` and `CMakeLists.txt`, runs the
  Python tests, builds the sdist and the universal wheel with uv and
  attaches them to the GitHub release, publishes them to PyPI through
  Trusted Publishing from the `pypi` environment, and then installs the
  released version from PyPI into a clean environment, imports it and checks
  `__version__`, one encoder and one parser. The README install line is
  `pip install bread-crumbs-contracts`.

### Changed

- CI and release workflows now build against CRUMBS `0.14.0` and linux-wire
  `0.1.3` (the version CRUMBS 0.14.0 builds with) while keeping the public
  package dependency range at `^0.12.0`.

## [0.4.5] - 2026-07-17

### Added

- Added the shared bus-liveness command-watchdog vocabulary (#14,
  feastorg/Slice_DCMT#6, feastorg/Slice_RLHT#5) in `bread_watchdog.h`:
  `BREAD_OP_SET_WATCHDOG` (0x7E, `[timeout_ms:u16]`, 0 = disarm) and
  `BREAD_OP_GET_WATCHDOG` (0x7D,
  `[armed:u8][timeout_ms:u16][tripped:u8][trip_count:u8]`) with
  `bread_watchdog_result_t`, `bread_watchdog_parse_payload()`, and
  `bread_watchdog_build_reply()`. Per-type wrappers
  `dcmt_send_set_watchdog()` / `dcmt_query_watchdog()` /
  `dcmt_get_watchdog()` and the RLHT equivalents, gated by new capability
  flags `DCMT_CAP_CMD_WATCHDOG` (bit 5) and `RLHT_CAP_CMD_WATCHDOG`
  (bit 6). Firmware boots disarmed, so standalone/serial use is
  unaffected; controllers must feature-detect the flag before sending
  `BREAD_OP_SET_WATCHDOG`. GET_STATE layouts are unchanged.

## [0.4.4] - 2026-07-14

### Added

- Added a tracked installed-package consumer smoke fixture and wired CI to use
  it after installing `bread-crumbs-contracts`, CRUMBS, and linux-wire.
- Added `dcmt_parse_state_payload()` and `rlht_parse_state_payload()`, the
  GET_STATE payload parsers previously inlined in `dcmt_get_state()` /
  `rlht_get_state()`. Controllers that run the query round-trip through their
  own transport (retry, locking, configurable delay) can now parse replies
  through the contracts instead of duplicating the wire layout. The `_get_*`
  round-trip helpers delegate to the new parsers with no behavior change.
- Extended the compile smoke test to exercise both state parsers at runtime
  (valid payload round-trip, short-payload and NULL rejection).

## [0.4.3] - 2026-06-11

### Fixed

- Updated bundled Linux controller examples to print the current fixed-layout DCMT state fields (`m1_pwm`, `m2_pwm`, `sp1`, `sp2`, `pos1`, `pos2`, `spd1`, `spd2`) instead of removed `target1`, `target2`, `value1`, and `value2` fields.
- Display sentinel-only DCMT state fields as `n/a` in controller examples when they contain `BREAD_INVALID_I16`.
- Aligned CMake, PlatformIO, and Arduino package metadata on version `0.4.3`.

### Added

- Added a tracked compile/link smoke target for `tests/compile_smoke/smoke.c`.
- Added CI coverage for both bundled Linux controller examples.

### Changed

- CI and release workflows now verify linux-wire and CRUMBS release artifact checksums before extraction.
- CI and release workflows now build against CRUMBS `0.12.4` while keeping the public package dependency range at `^0.12.0`.
- Release workflow now runs the compile/link smoke test before packaging.

## [0.4.2] - 2026-04-20

### Fixed

- Guard `find_package(crumbs)` with `if(NOT TARGET crumbs AND NOT TARGET crumbs::crumbs)` so that consumers using `add_subdirectory` to bring in both CRUMBS and bread-crumbs-contracts (e.g. anolis-provider-bread) no longer fail when the `crumbs` target is already in scope from the parent build.

## [0.4.0] - 2026-03-18

### Added

- `BREAD_INVALID_I16` (`-32768` / `INT16_MIN`) sentinel constant in `bread_caps.h`: protocol-wide encoding for "field not applicable in current mode or build configuration".
- `BREAD_INVALID_U8` (`0xFF`) sentinel for unsigned 8-bit fields.
- `BREAD_IS_VALID_I16(v)` and `BREAD_IS_VALID_U8(v)` helper macros for side-effect-free validity checks.

### Breaking (0.4.0)

- `dcmt_state_result_t` fields renamed and expanded:
  - Removed `target1`, `target2`, `value1`, `value2` (mode-normalized abstraction).
  - Added `m1_pwm`, `m2_pwm`, `sp1`, `sp2`, `pos1`, `pos2`, `spd1`, `spd2` as explicit raw fields matching the wire layout.
  - `sp1`/`sp2` will be `BREAD_INVALID_I16` when `mode == DCMT_MODE_OPEN_LOOP`.
  - `spd1`/`spd2` will be `BREAD_INVALID_I16` unless `mode == DCMT_MODE_CLOSED_SPEED`.
  - Use `BREAD_IS_VALID_I16()` before consuming these fields.
- `dcmt_get_state` parser now performs a direct byte-to-struct copy; mode-branching interpretation removed. Callers are responsible for field validity checks.

## [0.3.0] - 2026-03-18

### Changed

- Replaced docs set with canonical, task-oriented names and rewritten content:
  - `docs/overview.md`
  - `docs/contract-model.md`
  - `docs/capabilities.md`
  - `docs/controller-compatibility.md`
  - `docs/type-ids.md`
  - `docs/protocol-versioning.md`
- Removed legacy planning/policy doc names from `docs/` and aligned README links/layout.
- Updated DCMT capability documentation to reflect additive level semantics:
  - Level 2: closed-position + PID
  - Level 3: Level 2 + closed-speed
- Updated controller compatibility matrix for DCMT MCU performance profiles (classic Nano vs Nano Every).
- Standardized DCMT `GET_STATE` parsing support for fixed-layout payloads via explicit field offsets/length constants.

### Breaking

- Enforced fixed-layout DCMT `GET_STATE` parsing in `dcmt_get_state`; legacy variable-length payload parsing has been removed.

## [0.2.0] - 2026-03-14

### Added (0.2.0)

- Shared capability contract header: `include/bread/bread_caps.h`.
- Canonical `GET_CAPS` contract support in device headers:
  - `dcmt_query_caps` / `dcmt_get_caps`
  - `rlht_query_caps` / `rlht_get_caps`
- Capability constants for DCMT and RLHT (levels + flags).
- Controller examples now expose `caps` commands and capability-aware command gating.

### Breaking (0.2.0)

- Removed generation-scoped include paths from the public API:
  - `include/bread/gen1/*`
  - `include/bread/gen2/*`
  - `include/bread/gen3/*`

### Changed (0.2.0)

- Public include surface is now generation-free and capability-oriented.
- `bread_ops.h` now includes canonical top-level headers:
  - `bread_caps.h`
  - `bread_version_helpers.h`
  - `dcmt_ops.h`
  - `rlht_ops.h`
- README and docs aligned to capability-first model.

## [0.1.0] - 2026-03-10

### Added (0.1.0)

- Initial repository bootstrap.
- Header-only contract layout under `include/bread/`.
- Family include header `bread_ops.h`.
- Device contract headers:
  - `dcmt_ops.h`
  - `rlht_ops.h`
- Shared version parse and compatibility helpers.
- Governance docs for versioning, type IDs, and compatibility policy.
- Package metadata for PlatformIO/Arduino/CMake consumers.
