# Compatibility Matrix

This matrix is capability-oriented and device-centric.

## Capability Profiles (Current)

| Device | Level | Meaning |
| --- | --- | --- |
| DCMT | 1 | Open-loop + brake baseline |
| DCMT | 2 | Closed-loop position/speed support |
| DCMT | 3 | Reserved for future advanced control/diagnostics |
| RLHT | 1 | Baseline RLHT control/state contract |

## Expected Hardware Mapping (Informational)

| Hardware Build | Expected Profile |
| --- | --- |
| DCMT Gen1 | DCMT level 1 |
| DCMT Gen2 (open profile) | DCMT level 1 |
| DCMT Gen2 (closed profile) | DCMT level 2 |
| RLHT Gen1 | RLHT level 1 |
| RLHT Gen2 | RLHT level 1 |

Notes:

- Mapping is informational; controllers should trust runtime `GET_CAPS`, not assumed generation.
- If `GET_CAPS` is unavailable, controllers use documented baseline fallback per device type.
