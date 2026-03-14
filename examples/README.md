# BREAD Controller Examples

Linux controller examples for testing BREAD family slices via CRUMBS.

## Available Controllers

- `controller_discovery`: scans the I2C bus and discovers RLHT/DCMT automatically.
- `controller_manual`: uses a fixed device list from `config.h`.

Both provide an interactive shell for sending commands and reading state.
