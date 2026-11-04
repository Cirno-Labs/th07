"""Require whole-file executable identity, including headers and padding."""
from __future__ import annotations

import argparse
import hashlib
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
import project


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("rebuilt", nargs="?", type=Path, default=ROOT / project.EXE)
    parser.add_argument("original", nargs="?", type=Path, default=ROOT / project.ORIGINAL)
    args = parser.parse_args()
    rebuilt = args.rebuilt.read_bytes()
    original = args.original.read_bytes()
    differences = sum(a != b for a, b in zip(rebuilt, original)) + abs(len(rebuilt) - len(original))
    if differences:
        first = next((i for i, (a, b) in enumerate(zip(rebuilt, original)) if a != b),
                     min(len(rebuilt), len(original)))
        print(f"FAIL: {differences:,} bytes differ; first at file offset {first:#x}")
        return 1
    print(f"100% executable parity: {len(rebuilt):,} identical bytes; "
          f"SHA-1 {hashlib.sha1(rebuilt).hexdigest()}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
