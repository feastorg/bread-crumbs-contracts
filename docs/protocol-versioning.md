# Protocol Versioning

## SemVer Policy

This repository follows semantic versioning for wire contracts and helper APIs.

- MAJOR: breaking wire change or incompatible contract behavior
- MINOR: backward-compatible protocol additions
- PATCH: non-breaking fixes (docs/tests/helper internals)

## Device Version Reply Contract

Version query reply (`opcode 0x00`) payload:

- `[crumbs_version:u16][module_major:u8][module_minor:u8][module_patch:u8]`

Device headers expose module constants:

- `<DEVICE>_MODULE_VER_MAJOR`
- `<DEVICE>_MODULE_VER_MINOR`
- `<DEVICE>_MODULE_VER_PATCH`

## Capability and Version Interaction

Compatibility decisions should use both:

1. version query for broad compatibility checks,
2. capability flags for precise feature gating.

A device with newer minor version may still be compatible if required flags are present.

## Breaking-Change Rule

Reinterpreting existing opcode or payload semantics is a breaking change and requires:

- MAJOR version bump, and/or
- new type ID if split is unavoidable.
