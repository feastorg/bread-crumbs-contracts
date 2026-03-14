# BREAD Discovery Controller

Auto-discovery Linux controller for BREAD family slices.

## Features

- Scans bus for CRUMBS devices (`0x08..0x77`)
- Filters/uses BREAD types (`RLHT_TYPE_ID`, `DCMT_TYPE_ID`)
- Queries version (`opcode 0x00`) and checks compatibility
- Interactive commands for RLHT and DCMT

## Build

```bash
cd examples/controller_discovery
mkdir -p build && cd build
cmake ..
make
```

Default CRUMBS path assumes sibling repo at `../CRUMBS` from repo root.
Override with:

```bash
cmake .. -DCRUMBS_PATH=/path/to/CRUMBS
```

## Run

```bash
./controller_discovery
# or
./controller_discovery /dev/i2c-1
```
