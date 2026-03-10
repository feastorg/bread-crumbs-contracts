# Versioning Policy

This repository versions the BREAD wire contract, not firmware implementations.

## Semver Rules

- MAJOR: breaking wire change
  - Examples: opcode reuse, payload reinterpretation, removed operation
- MINOR: backward-compatible addition
  - Examples: new opcode, additive response field at a new opcode
- PATCH: non-wire change
  - Examples: docs updates, comments, metadata, CI changes

## Module Version Macros

Each device ops header exposes module version macros:

- `*_MODULE_VER_MAJOR`
- `*_MODULE_VER_MINOR`
- `*_MODULE_VER_PATCH`

Controllers should query opcode `0x00` and use compatibility checks before assuming advanced behavior.
