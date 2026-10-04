"""Loads tests/golden_vectors/vectors.json once for the test modules."""

import json
from pathlib import Path
from typing import Any

REPO_ROOT = Path(__file__).resolve().parents[3]
HEADER_DIR = REPO_ROOT / "include" / "bread"
VECTORS_PATH = REPO_ROOT / "tests" / "golden_vectors" / "vectors.json"

VECTORS: dict[str, Any] = json.loads(VECTORS_PATH.read_text(encoding="utf-8"))
CONSTANTS: dict[str, int] = VECTORS["constants"]
ENCODE: list[dict[str, Any]] = VECTORS["encode"]
PARSE: list[dict[str, Any]] = VECTORS["parse"]


def encode_id(vec: dict[str, Any]) -> str:
    args = ",".join(f"{k}={v}" for k, v in vec["args"].items())
    return f"{vec['name']}({args})"


def parse_id(vec: dict[str, Any]) -> str:
    return f"{vec['name']}({vec['payload'] or 'empty'})"
