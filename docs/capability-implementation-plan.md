# Capability-Oriented Implementation Plan

## Purpose

Define the full path from current state to a stable long-term BREAD contract architecture based on:

- generation-free public contract headers,
- additive capability signaling,
- controller runtime gating,
- minimal break risk during rollout.

This plan is execution-focused and intended to be followed in order.
As of the `0.2.0` cutover, Phases 0-4 for DCMT/RLHT are implemented; this document remains as the long-term reference pattern for future slices.

## Current State (As-Is)

- Contracts repo has canonical per-device headers (`dcmt_ops.h`, `rlht_ops.h`) plus shared capability helpers (`bread_caps.h`).
- Public generation-scoped include wrappers are removed from the contract surface.
- Version query (`opcode 0x00`) and standardized capability query (`BREAD_OP_GET_CAPS`) are in place.
- DCMT and RLHT firmware implement caps replies with additive level/flags mapping.
- Discovery/manual controller examples query and display caps and gate advanced commands by capability flags.

## Desired End State (To-Be)

- One canonical contract line per slice type.
- Public include surface is generation-free.
- Shared `BREAD_OP_GET_CAPS` query supported across slices.
- Capability payload v1 standardized as:
  - `[caps_schema:u8][caps_level:u8][caps_flags:u32_le]`
- Controllers use `version -> caps -> command gating` flow.
- Legacy fallback path documented and implemented.
- New type IDs are used only for true wire breaks.

## Non-Goals

- No per-generation contract forks for DCMT/RLHT unless wire-breaking divergence occurs.
- No protocol redesign beyond capability signaling and gating.

## Design Decisions Locked

1. Capability model is additive only.
2. Existing opcode/payload meaning cannot be reinterpreted under a capability flag.
3. `caps_level` is convenience; `caps_flags` is authoritative.
4. Missing/unsupported `GET_CAPS` maps to baseline capability fallback per type.
5. Split to new type ID only on true wire-breaking divergence.

## Execution Phases

## Phase 0: Public Surface Cleanup (Required)

Scope:

- Remove generation-scoped contract include paths from public API.
- Remove generation wrapper headers and references from docs/examples.
- Keep only canonical includes under `include/bread/*.h`.

Deliverables:

- `include/bread/gen*/` removed from published surface.
- `bread_gen1_ops.h` / `bread_gen2_ops.h` / `bread_gen3_ops.h` removed from published surface (if present).
- `bread_ops.h` includes only canonical device headers.
- Docs/examples updated to canonical includes.

Exit Criteria:

- No docs/examples/public headers reference generation include paths.
- Smoke compile passes with canonical includes only.

## Phase 1: Contract Surface Definition (Caps Constants + Helpers)

Scope:

- Define `BREAD_OP_GET_CAPS` constant and capability payload v1 constants.
- Define shared parse helpers for caps payload.
- Reserve and document capability flag bit ranges per device.

Deliverables:

- New shared header (example: `include/bread/bread_caps.h`).
- Updated docs with exact opcode and payload format.
- Initial capability flag registry tables for DCMT and RLHT.

Exit Criteria:

- Headers compile in smoke tests.
- All docs agree on one payload format and fallback behavior.

## Phase 2: Contracts Header Integration

Scope:

- Add `*_query_caps` and `*_get_caps` helpers to device headers.
- Add typed result structs for capabilities.
- Keep current API stable; only additive helper APIs.

Deliverables:

- `dcmt_ops.h` capability helpers and DCMT cap flag constants.
- `rlht_ops.h` capability helpers and RLHT cap flag constants.
- Family-level helper exposure via canonical `bread_ops.h` include.

Exit Criteria:

- Controller examples compile using new helper APIs.
- No behavior regression in existing version/state helper paths.

## Phase 3: Firmware Support (DCMT + RLHT)

Scope:

- Register reply handler for `BREAD_OP_GET_CAPS` in each slice firmware.
- Return v1 payload from firmware.
- DCMT flags reflect actual build profile (open-only vs closed-capable).
- RLHT returns stable baseline profile.

Deliverables:

- DCMT firmware reply implementation for caps.
- RLHT firmware reply implementation for caps.
- Firmware-side cap mapping notes in comments/docs.

Exit Criteria:

- Hardware returns expected level/flags for each firmware variant.
- Existing ops continue working unchanged.

## Phase 4: Controller Example Cutover

Scope:

- Update discovery/manual examples to query caps after version.
- Gate advanced commands strictly by cap flags.
- Add clear console output for unsupported features.

Deliverables:

- Updated `examples/controller_discovery/*`.
- Updated `examples/controller_manual/*`.
- Demo flows for DCMT open-only, DCMT closed-capable, RLHT baseline.

Exit Criteria:

- Controller can safely interact with all current profiles without generation branching.
- Unsupported commands are blocked with explicit messages.

## Phase 5: Validation + Release

Scope:

- Run compile checks for contracts + examples.
- Validate live hardware behavior against expected profiles.
- Tag and publish contracts package version with release notes.

Deliverables:

- Test record (compile + runtime matrix).
- Updated changelog with capability milestone.
- Published package version.

Exit Criteria:

- No protocol regressions against current slices.
- Capability flow demonstrated end-to-end.

## Capability Registry Seed (Initial)

## DCMT

Levels:

- Level 1: open-loop + brake baseline.
- Level 2: closed-loop position/speed control support.
- Level 3: reserved future diagnostics/tuning extensions.

Flags (initial seed):

- `DCMT_CAP_OPEN_LOOP_CONTROL`
- `DCMT_CAP_BRAKE_CONTROL`
- `DCMT_CAP_CLOSED_LOOP_POSITION`
- `DCMT_CAP_CLOSED_LOOP_SPEED`
- `DCMT_CAP_PID_TUNING`

## RLHT

Levels:

- Level 1: baseline RLHT contract.

Flags (initial seed, exact names to finalize in implementation PR):

- baseline control/state support flag(s) aligned with current RLHT operations.

## Test Matrix

Minimum matrix before release:

- DCMT Gen1 firmware -> expected DCMT level 1 flags.
- DCMT Gen2 open profile -> expected DCMT level 1 flags.
- DCMT Gen2 closed profile -> expected DCMT level 2 flags.
- RLHT Gen1 firmware -> expected RLHT level 1 flags.
- RLHT Gen2 firmware -> expected RLHT level 1 flags.
- Legacy/no-caps behavior -> controller baseline fallback path verified.

For each matrix entry:

1. Query version.
2. Query caps.
3. Execute supported command set.
4. Confirm unsupported command gating behavior.

## Risk Register and Mitigations

1. Risk: Capability drift between docs and firmware.
- Mitigation: keep bit registry in contracts repo as source of truth; CI compile/tests on examples.

2. Risk: Include-path break for consumers using generation wrappers.
- Mitigation: make surface cleanup explicit in release notes and changelog; update all first-party examples in same change.

3. Risk: Controller assumes generation instead of runtime caps.
- Mitigation: enforce runtime gating in examples and docs; no generation-based command branching.

4. Risk: Silent firmware fallback behavior hides unsupported ops.
- Mitigation: controllers gate commands before send; firmware still validates input safely.

5. Risk: Premature type-id splits create maintenance overhead.
- Mitigation: strict split criteria documented; add-only first.

## Rollback Strategy

If rollout causes issues:

- Keep existing version/state paths unchanged.
- Mark `GET_CAPS` as experimental in release notes.
- Fallback controllers to baseline behavior when caps query fails.
- Delay capability-gated commands until firmware alignment is restored.

## Ownership and Sequencing

Recommended order:

1. contracts repo surface cleanup + docs,
2. contracts caps constants/helpers,
3. DCMT firmware caps implementation,
4. RLHT firmware caps implementation,
5. controller example cutover,
6. validation matrix + package publish.

This order minimizes runtime ambiguity and keeps a clear source-of-truth workflow.

## Definition of Done

The migration is complete when all are true:

- Contracts expose stable `GET_CAPS` helpers and flag constants.
- DCMT and RLHT firmware return valid caps payloads.
- Controllers gate advanced commands by caps (not generation).
- Docs, examples, and published package are aligned.
- Generation-scoped contract include paths are removed from public surface.
