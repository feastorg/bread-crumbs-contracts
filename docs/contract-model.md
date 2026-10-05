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

## Payload Layouts

Each device header states its type, opcodes and payload layouts once, with
CRUMBS' `crumbs_ops.h` (CRUMBS 0.14.0 or newer):

- `CRUMBS_DEFINE_FAMILY(RLHT, 0x01, RLHT_OPS)` declares `RLHT_TYPE_ID` and the
  `RLHT_OP_*` constants; the build fails on a duplicate opcode.
- `CRUMBS_DEFINE_PAYLOAD(rlht_set_setpoints, 4, RLHT_SET_SETPOINTS_FIELDS)`
  declares `rlht_set_setpoints_t`, `rlht_set_setpoints_wire_size`,
  `rlht_set_setpoints_pack()` and `rlht_set_setpoints_unpack()`; the build
  fails if the field list does not sum to the stated size.

The controller wrappers (`rlht_send_*`, `dcmt_send_*`) pack through these
structs, and the state parsers unpack through `rlht_state` / `dcmt_state`.
Slice firmware unpacks each SET payload with the matching `*_unpack()` and
builds its GET_STATE reply with `rlht_state_pack()` / `dcmt_state_pack()`,
so both sides read one statement of every layout.

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
