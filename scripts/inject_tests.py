#!/usr/bin/env python3
"""Inject `add_subdirectory(tests)` into a checked-out amnezia-client's
client/CMakeLists.txt.

The test suite is built as a subdirectory of the client so that it inherits
the parent build's ${SOURCES}, ${HEADERS}, ${LIBS} and ${CLIENT_ROOT_DIR}.
The hook MUST be inserted *before* `main.cpp` is appended to ${SOURCES} —
otherwise test_common (built from ${SOURCES}) would pull in the app's main()
and clash with each test's QTEST_MAIN.

Idempotent: a no-op if the hook is already present.
"""
import sys

ANCHOR = "list(APPEND SOURCES ${CMAKE_CURRENT_LIST_DIR}/main.cpp)"
HOOK = (
    "if(NOT IOS AND NOT ANDROID AND NOT MACOS_NE)\n"
    "    add_subdirectory(tests)\n"
    "endif()\n\n"
)


def main(path: str) -> int:
    with open(path, "r", encoding="utf-8") as fh:
        text = fh.read()

    if "add_subdirectory(tests)" in text:
        print(f"[inject_tests] hook already present in {path}; nothing to do")
        return 0

    if ANCHOR not in text:
        print(f"[inject_tests] ERROR: anchor not found in {path!r}:\n  {ANCHOR}", file=sys.stderr)
        return 1

    text = text.replace(ANCHOR, HOOK + ANCHOR, 1)
    with open(path, "w", encoding="utf-8") as fh:
        fh.write(text)
    print(f"[inject_tests] inserted add_subdirectory(tests) into {path}")
    return 0


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("usage: inject_tests.py <path-to-client/CMakeLists.txt>", file=sys.stderr)
        sys.exit(2)
    sys.exit(main(sys.argv[1]))
