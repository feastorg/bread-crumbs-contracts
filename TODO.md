# TODO

Publication/readiness cleanup items found during CRUMBS paper review.

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
