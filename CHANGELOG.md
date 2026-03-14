# Changelog

## [Unreleased]

### Added

- Shared capability contract header: `include/bread/bread_caps.h`.
- Canonical `GET_CAPS` contract support in device headers:
  - `dcmt_query_caps` / `dcmt_get_caps`
  - `rlht_query_caps` / `rlht_get_caps`
- Capability constants for DCMT and RLHT (levels + flags).
- Controller examples now expose `caps` commands and capability-aware command gating.

### Changed

- Public include surface is now generation-free and capability-oriented.
- `bread_ops.h` now includes canonical top-level headers:
  - `bread_caps.h`
  - `bread_version_helpers.h`
  - `dcmt_ops.h`
  - `rlht_ops.h`
- README and docs aligned to capability-first model.

### Removed

- Generation-scoped public contract include tree:
  - `include/bread/gen1/*`
  - `include/bread/gen2/*`
  - `include/bread/gen3/*`

## [0.2.0] - 2026-03-14

### Added

- Capability-oriented contract rollout and generation-free header surface.

### Breaking

- Removed generation-scoped include paths from the public API.

## [0.1.0] - 2026-03-10

### Added

- Initial repository bootstrap.
- Header-only contract layout under `include/bread/`.
- Family include header `bread_ops.h`.
- Device contract headers:
  - `dcmt_ops.h`
  - `rlht_ops.h`
- Shared version parse and compatibility helpers.
- Governance docs:
  - `docs/versioning.md`
  - `docs/type-id-registry.md`
  - `docs/compatibility.md`
- Package metadata for PlatformIO/Arduino/CMake consumers.
