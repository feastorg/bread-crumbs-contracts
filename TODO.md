# TODO

Publication/readiness cleanup items found during CRUMBS paper review.

## Build and CI

- [ ] Fix `examples/controller_discovery` DCMT state printing after the `0.4.0` state-contract change.
  - Replace stale `target1`, `target2`, `value1`, `value2` references with current explicit fields:
    - `m1_pwm`, `m2_pwm`
    - `sp1`, `sp2`
    - `pos1`, `pos2`
    - `spd1`, `spd2`
  - Use `BREAD_IS_VALID_I16()` before printing sentinel-eligible fields.

- [ ] Fix `examples/controller_manual` DCMT state printing after the `0.4.0` state-contract change.
  - Same field update and sentinel handling as `controller_discovery`.

- [ ] Add CI coverage for bundled Linux controller examples.
  - Build `examples/controller_discovery`.
  - Build `examples/controller_manual`.
  - Ensure CI catches future contract/example drift.

- [ ] Add or promote a real compile/link smoke target for `tests/compile_smoke/smoke.c`.
  - Current CI uses an inline smoke consumer.
  - Prefer reusing the tracked smoke source so repository tests and CI validate the same artifact.

## Version and Release Metadata

- [ ] Align version metadata.
  - `CMakeLists.txt` currently reports `0.4.2`.
  - `CHANGELOG.md` contains `0.4.2`.
  - `library.json` currently reports `0.4.1`.
  - `library.properties` currently reports `0.4.1`.

- [ ] Update CRUMBS dependency references.
  - CI/release currently fetch CRUMBS `0.12.1`.
  - Current CRUMBS release is `0.12.4`.
  - Confirm desired minimum supported CRUMBS version for contracts before changing package dependency ranges.

- [ ] Check whether `BREAD_MIN_CRUMBS_VERSION` should remain `1200` or move to a newer minimum.
  - Keep at `1200` only if all public contracts intentionally support CRUMBS `0.12.0+`.
  - Raise only if contracts rely on newer CRUMBS API/behavior.

## Supply Chain / Release Hardening

- [ ] Replace unchecked `curl | tar` dependency installs in CI with download + SHA256 verification.
  - linux-wire release artifact.
  - CRUMBS release artifact.

- [ ] Update release workflow dependency fetches to match CI after CRUMBS/linux-wire versions are confirmed.

- [ ] Consider adding a core package consumer smoke test that uses installed `bread::contracts`.
  - Build from installed bread-crumbs-contracts, installed CRUMBS, and installed linux-wire.
  - Include `<bread/bread_ops.h>`.
  - Exercise basic constants and compatibility helpers.

## Documentation and Paper Evidence

- [ ] Document the role of this repo as the BREAD domain-contract layer above CRUMBS.
  - CRUMBS provides the generic frame/protocol/device abstraction.
  - bread-crumbs-contracts provides BREAD slice type IDs, operation headers, capability flags, and compatibility policy.

- [ ] Add a short architecture diagram to docs showing:

  ```text
  BREAD slice contracts
      -> CRUMBS protocol/device abstraction
          -> Arduino/Linux HALs
              -> linux-wire on Linux / Wire on Arduino
  ```

- [ ] Capture at least one successful controller interaction log for RLHT and/or DCMT after examples are fixed.

- [ ] Identify which firmware/controller repos currently consume this package.

- [ ] Confirm which BREAD slices are mature enough to cite as CRUMBS application-pathway evidence.
