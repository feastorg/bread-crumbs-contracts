# Contract Model

## Core Model

BREAD contracts are device-centric and capability-oriented:

- one canonical contract header per device type (`dcmt_ops.h`, `rlht_ops.h`),
- one shared capability query contract (`bread_caps.h`),
- generation-free public include surface.

## Public API Boundary

Canonical includes:

- `<bread/bread_ops.h>`
- `<bread/dcmt_ops.h>`
- `<bread/rlht_ops.h>`
- `<bread/bread_caps.h>`

Generation-scoped public headers are intentionally excluded.

## Capability Principle

Behavior differences are represented as additive capabilities, not generation forks.
Controllers must gate optional behavior by runtime capability flags.

## Generation Labels

Hardware generation labels (Gen1/Gen2/Gen3) are inventory metadata only.
They are not protocol identity and must not drive controller command branching.

## When to Split a Contract Line

Create a new contract line (new type ID) only for true wire breaks, such as:

- incompatible opcode semantics,
- reinterpretation of existing payload bytes,
- non-additive behavior that cannot be safely gated by capabilities.

Until then, evolve the same type line additively.
