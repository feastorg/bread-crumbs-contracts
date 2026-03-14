# Versioning

This repository versions the BREAD wire contracts and controller-side helpers.

## SemVer Policy

- MAJOR: breaking wire change (or other incompatible contract behavior change).
- MINOR: backward-compatible additions (new ops, new capability flags, append-only payload extensions).
- PATCH: docs/tests/helper fixes with no protocol behavior break.

## Device Module Version Fields

Each device contract header exposes module version macros:

- `<DEVICE>_MODULE_VER_MAJOR`
- `<DEVICE>_MODULE_VER_MINOR`
- `<DEVICE>_MODULE_VER_PATCH`

Firmware should return these in `opcode 0x00` reply payload:

- `[CRUMBS_VERSION:u16][module_major:u8][module_minor:u8][module_patch:u8]`

## Capability Versioning

Capabilities are part of compatibility, not an afterthought.

Controllers should:

1. query `opcode 0x00`,
2. then query `BREAD_OP_GET_CAPS`,
3. gate behavior by returned capability flags.

`GET_CAPS` v1 payload:

- `[caps_schema:u8][caps_level:u8][caps_flags:u32_le]`

Rules:

- `caps_level` and `caps_flags` are additive.
- Unknown flags must be ignored by old controllers.
- Reinterpreting existing bytes is a breaking change (MAJOR/new type line).

## Surface Rule

Hardware generation labels are not protocol identifiers.
Public contract include paths must remain generation-free.
