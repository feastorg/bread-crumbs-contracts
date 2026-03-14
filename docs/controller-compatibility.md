# Controller Compatibility

## Required Runtime Flow

For each discovered device:

1. Query version (`opcode 0x00`).
2. Query capabilities (`BREAD_OP_GET_CAPS`).
3. Gate optional commands by returned flags.
4. Use baseline fallback if caps query is unavailable.

## Baseline Fallback Policy

If `GET_CAPS` fails:

- map by type ID to baseline capability set,
- disable optional/advanced operations,
- continue only with safe baseline operations.

## Compatibility Matrix (Current)

| Device Profile | Expected Cap Level |
| --- | --- |
| DCMT Gen1 firmware | DCMT level 1 |
| DCMT Gen2 open firmware | DCMT level 1 |
| DCMT Gen2 closed firmware | DCMT level 2 |
| RLHT Gen1 firmware | RLHT level 1 |
| RLHT Gen2 firmware | RLHT level 1 |

Generation mapping is informational only; runtime caps remain authoritative.

## Error Handling Expectations

Controllers should:

- report unsupported commands clearly,
- avoid sending commands behind missing capability flags,
- tolerate unknown capability flags.
