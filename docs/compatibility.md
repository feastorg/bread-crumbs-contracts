# Compatibility Guidance

## Controller Startup Flow

Recommended per-device startup checks:

1. Query opcode `0x00` (version reply).
2. Parse payload using `bread_parse_version()`.
3. Enforce minimum CRUMBS compatibility using `bread_check_crumbs_compat()`.
4. Enforce module compatibility using `bread_check_module_compat()` against the compiled header expectation.

## Compatibility Rule

- Module MAJOR must match exactly.
- Peripheral module MINOR should be >= controller expected MINOR.
- Module PATCH is ignored for protocol compatibility.

## Failure Handling

On incompatibility, controller should:

- mark device unavailable for normal operation,
- log mismatch details (type ID, observed versions, expected versions),
- avoid issuing state-changing commands to that device.
