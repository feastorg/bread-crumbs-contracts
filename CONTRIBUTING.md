# Contributing to bread-crumbs-contracts

This repository is two things that share one version: the header-only C
operation contracts in `include/bread/` (the `*_ops.h` family headers, caps,
watchdog and version helpers), and the pure-Python codec in
`bindings/python/`. A wire layout is declared once in a family header and the
Python codec mirrors it; both are checked against the golden vectors in
`tests/golden_vectors/vectors.json`, which the C generator writes.

## Build and test

### Python codec

Everything runs from the repo root with uv, as the `python` CI job does:

```sh
uv sync --locked            # the locked dev environment
uv run ruff format --check  # format
uv run ruff check           # lint
uv run pyright              # strict types
uv run pytest               # the codec against the golden vectors
uv build                    # the sdist and wheel
```

### C headers

The C build needs CRUMBS and linux-wire (CRUMBS's Linux HAL dependency) on
`CMAKE_PREFIX_PATH`. CI installs the pinned, checksum-verified release
tarballs named in `.github/workflows/ci.yml`; point `CMAKE_PREFIX_PATH` at
wherever you unpack the same two. Then:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="/path/to/crumbs-install;/path/to/linux-wire-install"
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

`vectors.json` is generated: the `bread_contracts_gen_vectors` target
rewrites it and CI fails on any difference, so regenerate and commit it when a
layout changes.

## Code style

- C11 headers under `include/bread`; the Python codec under
  `bindings/python/src`. Declare each wire layout once in its family header
  and let the Python codec and the golden vectors follow it, rather than
  hand-writing offsets on either side.
- Conventional Commits: `type(scope): subject`, imperative, lowercase, no
  trailing period, with `type` one of `feat`, `fix`, `docs`, `refactor`,
  `test`, `ci`, `chore`, `build`. A breaking wire or API change carries a `!`
  and a `BREAKING CHANGE:` footer.

## Pull requests

Branch from `main`, open a PR, and let CI finish: the single required check is
`ok`, which depends on every job. PRs are squash-merged with the PR body as
the commit message, so write the body as the commit you want in history. Add a
`[Unreleased]` entry to `CHANGELOG.md` for anything a user would notice.

## CI

`ci.yml` runs on pushes and PRs; the `ok` gate depends on:

| Job | What it proves |
| --- | --- |
| `cmake-package-smoke` | the C package builds, installs and passes `ctest`; a `find_package(bread_crumbs_contracts)` consumer builds against the install; the two controller examples build out of tree |
| `golden-vectors` | `tests/golden_vectors/vectors.json` is what the generator writes from the headers today |
| `python` | the codec is formatted, lint-clean, strictly typed, and passes its tests against those vectors on 3.11 and 3.13; the sdist and wheel build |

## Releasing

The tag must point at the commit on `main` that carries the version, and must
stay an ancestor of `main` (v0.7.0 did not: it was tagged on the prep-branch
commit and the PR squash-merged to a different one). So merge first, then tag.

1. On a branch, bump the version in `library.json`, `library.properties`,
   `pyproject.toml` and `CMakeLists.txt`, run `uv lock` so `uv.lock` records
   it, move the README Quick Start pin to the new `^X.Y.0`, and move the
   `[Unreleased]` CHANGELOG entries under a new `## [X.Y.Z] - YYYY-MM-DD`
   heading dated the day the tag will be made, in local time.
   `test_package_version_matches_tracked_file` checks all four files and
   `uv.lock` agree, so a missed file fails before a tag exists. Open the PR
   and let CI go green.
2. Squash-merge the PR to `main`.
3. Tag the resulting `main` commit and push the tag:

   ```sh
   git switch main && git pull
   git tag -a vX.Y.Z -m "Release vX.Y.Z"
   git push origin vX.Y.Z
   ```

   `release.yml` builds the GitHub release from the tag, then waits for a
   reviewer to approve the PyPI upload of `bread-crumbs-contracts` in the
   `pypi` environment; `verify-pypi` then installs it from PyPI and checks it.

4. Publish to the PlatformIO registry by hand, from a `git archive` of the tag
   — never the working tree, which `pio pkg publish` packs as it finds it,
   untracked files included:

   ```sh
   d=$(mktemp -d)
   git archive --format=tar vX.Y.Z | tar -x -C "$d"
   pio pkg publish "$d"
   ```

   A published version cannot be replaced. Then confirm it resolves:
   `pio pkg show cameronbrooks11/bread-crumbs-contracts@X.Y.Z`.

5. Confirm the tag is on `main`:
   `git merge-base --is-ancestor vX.Y.Z main && echo ok`.

6. Bump the consumers that pin this package. A PlatformIO caret on a 0.x
   version stops below the next minor (`^0.7.0` matches `>=0.7.0 <0.8.0`), so
   a minor release reaches no consumer on its own: the Slice firmware
   (`Slice_RLHT`, `Slice_DCMT`) and any other consumer bump their pin
   explicitly.
