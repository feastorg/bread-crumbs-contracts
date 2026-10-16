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

## Compatibility Rule

The module major is the wire-compatibility number, and capability flags
cover optional features. The version reply keeps its 5-byte layout.

- A wire-breaking change to a type bumps that type's module major
  (`RLHT_MODULE_VER_MAJOR`, `DCMT_MODULE_VER_MAJOR`). A controller refuses
  to actuate a device whose major differs from the one it was built
  against.
- An optional feature is a new bit in the existing u32 caps flags. Old
  firmware advertises no new bit, so each bit is defined so that its
  absence means the behaviour before the bit existed.
- Controllers key on the major and on caps flags, never on the module
  minor or patch.

`bread_check_module_compat()` applies the rule: it returns 0 when the
majors match and -1 when they differ, and ignores both minors. Before
0.7.0 it also returned -2 for a peripheral minor older than the expected
one.

## Breaking-Change Rule

Reinterpreting existing opcode or payload semantics is a breaking change and requires:

- a bump of that type's module major, and a package MAJOR bump (a MINOR
  bump while the package is 0.x), or
- new type ID if split is unavoidable.
