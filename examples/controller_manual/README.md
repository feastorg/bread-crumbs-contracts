# BREAD Manual Controller

Preconfigured Linux controller for BREAD family slices.

## Features

- Uses configured addresses in `config.h`
- Version probing/compatibility check via `probe` command
- Interactive commands for RLHT and DCMT

## Build

```bash
cd examples/controller_manual
mkdir -p build && cd build
cmake ..
make
```

Override CRUMBS path if needed:

```bash
cmake .. -DCRUMBS_PATH=/path/to/CRUMBS
```

## Run

```bash
./controller_manual
# or
./controller_manual /dev/i2c-1
```
