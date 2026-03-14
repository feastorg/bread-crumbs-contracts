# Compatibility Checklist

Controller-side compatibility flow for BREAD slices:

1. Query version (`opcode 0x00`).
2. Parse with shared version helpers.
3. Query capabilities (`BREAD_OP_GET_CAPS`).
4. Gate all non-baseline behavior by capability flags.

## Required Behavior

Controllers should:

- reject incompatible major version mismatches,
- allow compatible newer minor versions,
- assume baseline caps if `GET_CAPS` is unavailable (legacy fallback),
- avoid sending unsupported commands,
- degrade gracefully with clear unsupported-feature errors.

## Legacy Fallback

If `GET_CAPS` is unsupported/timeout:

- treat device as baseline capability profile for its type,
- do not assume advanced features,
- continue with safe operations only.

## Breaking Change Rule

If firmware changes semantics of existing opcodes/payload bytes, compatibility is broken.
That requires MAJOR bump and/or new type contract line.
