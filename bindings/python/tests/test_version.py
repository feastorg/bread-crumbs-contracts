"""The Python package carries the header package's version.

``pyproject.toml`` sets the distribution version ``__version__`` reports;
``library.json``, ``library.properties`` and ``CMakeLists.txt`` carry the
header package's. One release tag names all of them, so a bump that misses
a file fails here before it reaches the tag.
"""

import json
import re
import tomllib
from collections.abc import Callable

import pytest

import bread_crumbs_contracts as bcc

from vectors import REPO_ROOT

_CMAKE_PROJECT_VERSION = re.compile(r"^project\(.*\bVERSION[ \t]+(\S+)", re.M)
_SEMVER = re.compile(r"^\d+\.\d+\.\d+$")


def library_json_version() -> str:
    return json.loads((REPO_ROOT / "library.json").read_text(encoding="utf-8"))["version"]


def library_properties_version() -> str:
    for line in (REPO_ROOT / "library.properties").read_text(encoding="utf-8").splitlines():
        key, _, value = line.partition("=")
        if key == "version":
            return value
    raise AssertionError("library.properties has no version= line")


def pyproject_version() -> str:
    return tomllib.loads((REPO_ROOT / "pyproject.toml").read_text(encoding="utf-8"))["project"][
        "version"
    ]


def cmake_project_version() -> str:
    cmake_lists = (REPO_ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    match = _CMAKE_PROJECT_VERSION.search(cmake_lists)
    assert match, "CMakeLists.txt has no project(... VERSION ...) line"
    return match.group(1)


VERSION_SOURCES: list[Callable[[], str]] = [
    library_json_version,
    library_properties_version,
    pyproject_version,
    cmake_project_version,
]


def test_version_is_a_release_number() -> None:
    assert _SEMVER.match(bcc.__version__), bcc.__version__


@pytest.mark.parametrize("read", VERSION_SOURCES, ids=lambda read: read.__name__)
def test_package_version_matches_tracked_file(read: Callable[[], str]) -> None:
    assert read() == bcc.__version__
