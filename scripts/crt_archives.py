"""Prepare ordered partitions of the original CRT archive with MSVC lib.exe.

The common partition retains every original member except throw.obj, in its
original order. The tail partition contains that unmodified member. Outputs
and their hash manifest live under the ignored build directory. The project build hook prepares these library inputs before linking.
"""
from __future__ import annotations

import argparse
from collections.abc import Iterator
from dataclasses import dataclass
import hashlib
import json
import os
from pathlib import Path
import sys
from typing import Any

ROOT = Path(__file__).resolve().parents[1]


@dataclass(frozen=True)
class CrtArchives:
    common: Path
    throw: Path


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def archive_members(path: Path) -> Iterator[tuple[str, bytes]]:
    """Read member names and bytes, excluding the archive's symbol indexes."""
    data = path.read_bytes()
    if not data.startswith(b"!<arch>\n"):
        raise ValueError(f"Not a COFF archive: {path}")
    offset = 8
    names = b""
    while offset < len(data):
        header = data[offset:offset + 60]
        if len(header) != 60 or header[58:60] != b"`\n":
            raise ValueError(f"Invalid archive member at {offset:#x}: {path}")
        size = int(header[48:58])
        if size < 0:
            raise ValueError(f"Negative archive member size: {path}")
        body = data[offset + 60:offset + 60 + size]
        if len(body) != size:
            raise ValueError(f"Truncated archive member: {path}")
        name = header[:16].decode("ascii").strip()
        if name == "//":
            names = body
        elif name.startswith("/") and name[1:].isdigit():
            index = int(name[1:])
            ends = [end for end in (names.find(b"\0", index),
                                   names.find(b"/\n", index)) if end >= 0]
            if not ends:
                raise ValueError(f"Unterminated archive member name: {path}")
            yield names[index:min(ends)].decode("latin1"), body
        elif name != "/":
            yield name.rstrip("/"), body
        offset += 60 + size + size % 2


def local_tool_path(path: str, prefix: Path) -> Path:
    """Resolve this project's configured C: toolchain for native or Wine use."""
    if os.name != "nt" and path[:3].upper() == "C:\\":
        return prefix / "drive_c" / path[3:].replace("\\", "/")
    return Path(path)


def cache_matches(manifest: Path, inputs: dict[str, str], outputs: CrtArchives) -> bool:
    try:
        cached: Any = json.loads(manifest.read_text())
        return cached["inputs"] == inputs and all(
            cached["outputs"][path.name] == sha256(path)
            for path in (outputs.common, outputs.throw)
        )
    except (OSError, ValueError, KeyError, TypeError):
        return False


def prepare_archives(library: Path | None = None, output: Path | None = None) -> CrtArchives:
    """Generate and validate the partitions, reusing hash-verified outputs."""
    sys.path.insert(0, str(ROOT / "tools"))
    import build

    output = (output or ROOT / "build/crt_archives").resolve()
    if not output.is_relative_to(ROOT / "build"):
        raise ValueError("CRT archive outputs must stay under the ignored build directory")
    prefix = Path(build.ENV["WINEPREFIX"])
    tool = local_tool_path(build.host(str(build.BUILD["bin"]) + "\\lib.exe"), prefix)
    library = (library or tool.parent.parent / "lib/libcmt.lib").resolve()
    outputs = CrtArchives(output / "libcmt_ordered.lib", output / "libcmt_throw.lib")
    manifest = output / "manifest.json"
    inputs = {"library_sha256": sha256(library), "librarian_sha256": sha256(tool),
              "helper_sha256": sha256(Path(__file__))}
    if cache_matches(manifest, inputs, outputs):
        return outputs
    output.mkdir(parents=True, exist_ok=True)
    original = list(archive_members(library))
    throws = [(name, data) for name, data in original
              if name.replace("\\", "/").split("/")[-1].lower() == "throw.obj"]
    if len(throws) != 1:
        raise ValueError(f"Expected one original throw.obj, found {len(throws)}")
    throw_name, throw_bytes = throws[0]
    retained = [(name, data) for name, data in original if name != throw_name]
    throw_object = output / "throw.obj"
    throw_object.write_bytes(throw_bytes)
    unordered = output / "libcmt_unordered.lib"
    for generated in (unordered, outputs.common, outputs.throw):
        generated.unlink(missing_ok=True)

    def argument(path: Path) -> str:
        return build.win(str(path.relative_to(ROOT)) if path.is_relative_to(ROOT) else str(path))

    def librarian(*arguments: str) -> None:
        code, text = build.run_tool("lib", *arguments)
        if code:
            raise RuntimeError(f"MSVC librarian failed ({code}):\n{text}")

    librarian("/OUT:" + argument(unordered), "/REMOVE:" + throw_name, argument(library))
    # This librarian reverses an archive's member order when rewriting it.
    # A second tool invocation restores the original retained member order.
    librarian("/OUT:" + argument(outputs.common), argument(unordered))
    librarian("/OUT:" + argument(outputs.throw), argument(throw_object))
    if list(archive_members(outputs.common)) != retained:
        raise RuntimeError("Common CRT archive changed member order or member bytes")
    tail = list(archive_members(outputs.throw))
    if len(tail) != 1 or tail[0][1] != throw_bytes:
        raise RuntimeError("Tail CRT archive does not contain the intact original throw.obj")
    if sha256(library) != inputs["library_sha256"]:
        raise RuntimeError("The installed CRT archive changed during generation")
    manifest.write_text(json.dumps({"inputs": inputs, "members": len(retained),
                                   "outputs": {path.name: sha256(path)
                                               for path in (outputs.common, outputs.throw)}},
                                  indent=2, sort_keys=True) + "\n")
    return outputs


def main() -> None:
    os.chdir(ROOT)
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--library", type=Path, help="Installed original libcmt.lib")
    parser.add_argument("--output", type=Path, help="Output directory below build/")
    args = parser.parse_args()
    outputs = prepare_archives(args.library, args.output)
    print(json.dumps({"common": str(outputs.common.relative_to(ROOT)),
                      "throw": str(outputs.throw.relative_to(ROOT))}, sort_keys=True))


if __name__ == "__main__":
    main()
