# Hardware Generation Boundaries

## Purpose

Clarify how hardware generation labels relate to contracts.

## Rule

Generation labels (Gen1/Gen2/Gen3) are hardware lineage metadata only.
They are not part of the public protocol surface.

## Contract Boundary

Public contracts are device-capability based:

- `bread/dcmt_ops.h`
- `bread/rlht_ops.h`
- shared contract helpers (for example `bread_ops.h`, caps helpers)

Generation-scoped public include paths are intentionally excluded.

## What Generation Labels Are Still Used For

- hardware inventory and manufacturing context,
- test matrix labeling,
- rollout planning.

## What Generation Labels Must Not Be Used For

- controller command gating,
- public include path selection,
- type ID assignment by default.

## Split Trigger

If a future hardware generation introduces a true wire break that cannot be represented additively, introduce a new type contract line and document migration.

Until then, evolve by capabilities under the same device contract line.
