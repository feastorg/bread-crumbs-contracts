# Changelog

## [Unreleased]

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
