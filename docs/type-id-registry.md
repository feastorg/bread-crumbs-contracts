# Type ID Registry

Type IDs identify device contract lines.

## Allocations

- `0x01` -> RLHT
- `0x02` -> DCMT

## Policy

- Keep type IDs globally unique across this repository.
- Hardware generation labels are not a reason to assign a new type ID by themselves.
- Additive capability differences stay under the same type ID.

Assign a new type ID only for true wire-breaking divergence:

- incompatible opcode semantics,
- payload byte reinterpretation,
- non-additive protocol behavior.

When assigning a new ID:

- document it here,
- freeze old line behavior,
- document migration path.
