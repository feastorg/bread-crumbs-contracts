# Capability-Oriented Contract Policy

## Decision

BREAD contracts are **device-centric and capability-oriented**.

Public contract surface must be generation-free:

- Keep one canonical contract header per device (`dcmt_ops.h`, `rlht_ops.h`, etc.).
- Do not expose generation-scoped contract includes as part of the public API.
- Represent behavioral differences with additive capabilities.

This matches current reality:

- RLHT Gen1 and Gen2 share the same wire contract.
- DCMT Gen1 and Gen2-open share the same wire contract.
- DCMT closed-loop behavior is a feature superset, not a separate protocol line.

## Capability Signaling Model

Use a shared BREAD meta-query for all slices:

- `BREAD_OP_GET_CAPS` (requested via CRUMBS reply-query path)

### Reply payload v1

- `[caps_schema:u8][caps_level:u8][caps_flags:u32_le]`

Notes:

- `caps_level` is a simple additive tier for humans and quick policy checks.
- `caps_flags` is the authoritative machine gating mechanism.
- Unknown flags are ignored by older controllers.
- Future schemas may extend payload; schema byte gates parsing.

## Why Level + Flags (Hybrid)

`level` alone is eventually too rigid when features evolve non-linearly.

- Keep `level` for simplicity and UX.
- Keep `flags` for precise, future-proof gating.

Both remain additive.

## Core Rules

1. Capabilities are additive only.
2. Existing opcode and payload semantics never change meaning.
3. New behavior is added by new flags/opcodes/append-only fields.
4. If old bytes must be reinterpreted, that is a breaking change.

## Split Criteria (When to create a new contract line/type ID)

Create a new line only if one or more are true:

- Existing opcode semantics change incompatibly.
- Existing payload bytes are redefined.
- Required behavior cannot be represented additively.

When split occurs:

- assign a new type ID,
- freeze old contract behavior,
- document migration path.

## Controller Requirements

Controller flow:

1. Discover type ID.
2. Query version (`opcode 0x00`).
3. Query capabilities (`BREAD_OP_GET_CAPS`).
4. Gate commands by `caps_flags` (and optionally `caps_level`).

Legacy fallback:

- If `GET_CAPS` is unsupported/timeout, assume documented baseline capabilities for that type.

Controllers must not branch behavior by generation labels.

## Initial BREAD Mapping

### DCMT

Suggested capability levels:

- Level 1: open-loop + brake control.
- Level 2: closed-loop position/speed support + associated control ops.
- Level 3: advanced tuning/diagnostics (reserved for future).

Suggested flags (initial):

- `DCMT_CAP_OPEN_LOOP_CONTROL`
- `DCMT_CAP_BRAKE_CONTROL`
- `DCMT_CAP_CLOSED_LOOP_POSITION`
- `DCMT_CAP_CLOSED_LOOP_SPEED`
- `DCMT_CAP_PID_TUNING`

### RLHT

- Single unified contract line.
- Start with stable baseline level and flags.
- Extend additively as needed.

## Versioning Interaction

- Additive caps/ops => MINOR version bump.
- Breaking wire changes => MAJOR version bump (or new type line + docs).

Hardware generation labels are inventory context only, not protocol identity.

## Rollout Plan

1. Remove generation-scoped contract include paths from public surface.
2. Define shared caps constants and payload schema in contracts.
3. Add `GET_CAPS` helpers for each device header.
4. Implement firmware reply handlers in slices.
5. Update discovery/manual controllers to query and gate by caps.

## Bottom Line

Long-term stable path: **one contract line per slice type + additive capabilities**.
