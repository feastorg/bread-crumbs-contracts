# BREAD Contracts Docs

This documentation describes the public wire-contract model for BREAD slices.

## Scope

- Canonical public headers in `include/bread/`
- Capability discovery (`BREAD_OP_GET_CAPS`)
- Controller compatibility behavior
- Type-ID allocation and versioning policy

## Quick Start

1. Include canonical headers:
   - `#include <bread/bread_ops.h>`
2. Query device version (`opcode 0x00`).
3. Query device capabilities (`BREAD_OP_GET_CAPS`).
4. Gate optional commands by capability flags.

## Document Map

- `contract-model.md`: contract architecture and invariants
- `capabilities.md`: capability wire format and per-device capability registry
- `controller-compatibility.md`: runtime compatibility rules and fallback behavior
- `type-ids.md`: type-ID assignments and allocation policy
- `protocol-versioning.md`: semver and module-version policy
