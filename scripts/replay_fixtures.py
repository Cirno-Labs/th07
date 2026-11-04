"""Extract the three bundled replays from the user's th07.dat, into build/.

    python3 scripts/replay_fixtures.py
    just replay build/replay/fixtures/*.rpy

No game assets are added to the repository. These are single-stage runs,
not substitutes for the full-route corpus downloaded by replay.py fetch.
"""
from __future__ import annotations

from pathlib import Path
import struct

try:
    from .replay_hook import decompress
except ImportError:
    from replay_hook import decompress

ROOT = Path(__file__).resolve().parents[1]


def extract(archive: Path, destination: Path) -> list[Path]:
    data = archive.read_bytes()
    if len(data) < 16 or data[:4] != b"PBG4":
        raise ValueError(f"{archive}: not a PBG4 archive")
    count, table_offset, size = struct.unpack_from("<III", data, 4)
    if not 16 <= table_offset < len(data):
        raise ValueError("invalid archive table offset")
    table = decompress(data[table_offset:], size)
    entries: list[tuple[str, int, int]] = []
    at = 0
    for _ in range(count):
        end = table.index(0, at)
        name = table[at:end].decode("shift_jis")
        offset, length, _ = struct.unpack_from("<III", table, end + 1)
        entries.append((name, offset, length))
        at = end + 13
    paths = []
    for i, (name, offset, length) in enumerate(entries):
        if not name.lower().endswith(".rpy"):
            continue
        end_offset = entries[i + 1][1] if i + 1 < len(entries) else table_offset
        if not 16 <= offset < end_offset <= table_offset:
            raise ValueError(f"invalid entry {name}")
        path = destination / name.replace("\\", "/").rsplit("/", 1)[-1]
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(decompress(data[offset:end_offset], length))
        paths.append(path)
    return paths


if __name__ == "__main__":
    for path in extract(ROOT / "orig/th07_100b/th07.dat", ROOT / "build/replay/fixtures"):
        print(path.relative_to(ROOT))
