# Type IDs

Type IDs identify contract lines.

## Allocations

- `0x01`: RLHT
- `0x02`: DCMT

## Allocation Rules

- Type IDs must be unique within this repository.
- Hardware generation labels alone do not justify a new type ID.
- Additive capability differences stay on the same type ID.

## New Type-ID Criteria

Assign a new type ID only for wire-breaking divergence:

- incompatible opcode semantics,
- reinterpretation of existing payload bytes,
- required non-additive behavior.

When assigning a new ID, document the migration path and freeze legacy behavior.
