"""Parser and harness integration checks, without Wine or network access."""
from __future__ import annotations

from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
from threading import Event
from types import ModuleType, SimpleNamespace
import hashlib
import json
import struct
import sys
import tempfile
import unittest
from unittest.mock import patch

from scripts import replay_hook as hook
from scripts.replay_fixtures import ROOT, extract

sys.path.insert(0, str(ROOT / "tools/replay"))
import replay


def literals(data: bytes) -> bytes:
    """Encode literals and the end marker independently of the decoder."""
    bits = "".join("1" + f"{value:08b}" for value in data) + "0" * 14
    bits += "0" * (-len(bits) % 8)
    return int(bits, 2).to_bytes(len(bits) // 8, "big")


def encode(data: bytearray) -> bytes:
    packed = literals(data[0x54:])
    struct.pack_into("<III", data, 0x10, len(data), len(packed), len(data) - 0x54)
    file = data[:0x54] + packed
    file[0xd] = 0x35
    struct.pack_into("<I", file, 8, (0x3f000318 + sum(file[0xd:])) & 0xffffffff)
    key = file[0xd]
    for i in range(0x10, len(file)):
        file[i] = (file[i] + key) & 255
        key = (key + 7) & 255
    return bytes(file)


def decoded(difficulty: int = 1) -> bytearray:
    # Three gameplay calcs, the leading empty frame and TWO final markers.
    data = bytearray(0xe8 + 0x2c + 6 * 4 + 2)
    data[:4] = b"T7RP"
    struct.pack_into("<H", data, 4, 0x1100)
    idx = 6 if difficulty == 5 else 0
    struct.pack_into("<I", data, 0x1c + 4 * idx, 0xe8)
    struct.pack_into("<I", data, 0x38 + 4 * idx, len(data) - 2)
    data[0x56:0x58] = bytes([4, difficulty])
    data[0x58:0x5e] = b"11/04\0"
    data[0x5e:0x62] = b"TEST"
    struct.pack_into("<I", data, 0x6c, 123)
    struct.pack_into("<I", data, 0xe8, 123)
    struct.pack_into("<HBBB", data, 0xe8 + 0x20, 0xabcd, 128, 2, 3)
    return data


FIELDS = [*replay.REQUIRED_FIELDS, "entry"]


def trace(score: int = 123, frames: int = 3, end_entry: int = 4,
          title: bool = True) -> replay.Trace:
    records: list[tuple[int, ...]] = []

    def add(kind: int, **values: int) -> None:
        base = dict(kind=kind, seq=len(records), passes=len(records) + 10,
                    calcs=len(records) + 10, stage=1)
        base.update(values)
        records.append(tuple(base.get(field, 0) for field in FIELDS))

    add(replay.STAGE_START)
    for frame in range(1, frames + 1):
        add(replay.CALC, replayFrame=frame, entry=frame, flags=1)
    add(replay.REPLAY_END, replayFrame=frames, entry=end_entry, score=score)
    if title:
        add(replay.TITLE)
    return replay.Trace(FIELDS, "test", records)


class ReplayTest(unittest.TestCase):
    def setUp(self) -> None:
        directory = tempfile.TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        self.path = Path(directory.name, "test.rpy")
        self.path.write_bytes(encode(decoded()))

    def test_header_and_inputs(self) -> None:
        record = hook.Replay(self.path)
        self.assertEqual(record.label, "SakuyaA Normal")
        self.assertEqual(record.score, 1230)
        self.assertEqual(record.name, "TEST")
        self.assertEqual(record.start(), "0,1,4")
        self.assertEqual(record.stages[1], dict(score=123, frames=3, frame_counts=[3, 4], entries=6,
                                               seed=0xabcd, lives=2, bombs=3, power=128))

    def test_downloaded_corpus(self) -> None:
        manifest = ROOT / "replays/manifest.json"
        if not manifest.exists():
            self.skipTest("downloaded replay corpus is optional")
        entries = json.loads(manifest.read_text())["replays"]
        self.assertEqual(len(entries), 36)
        for entry in entries:
            path = ROOT / "replays" / entry["file"]
            with self.subTest(replay=path.name):
                self.assertEqual(hashlib.sha1(path.read_bytes()).hexdigest(), entry["sha1"])
                record = hook.Replay(path)
                self.assertEqual(record.label, f"{entry['shot']} {entry['difficulty']}")
                self.assertEqual(record.score, entry["score"])
                self.assertTrue(all(stage["frames"] > 0 for stage in record.stages.values()))

    def test_multiple_stages(self) -> None:
        data = decoded()
        final = data[0xe8:-2]
        block = final[:-4]  # earlier stages get only one EndStage call
        data = data[:0xe8] + block + final + bytes(4)
        struct.pack_into("<I", data, 0x20, 0xe8 + len(block))
        struct.pack_into("<II", data, 0x38, len(data) - 4, len(data) - 2)
        struct.pack_into("<I", data, 0xe8 + len(block), 456)
        self.path.write_bytes(encode(data))
        record = hook.Replay(self.path)
        self.assertEqual(list(record.stages), [1, 2])
        self.assertEqual([stage["score"] for stage in record.stages.values()], [123, 456])
        self.assertEqual([stage["frames"] for stage in record.stages.values()], [3, 3])

    def test_phantasm_is_stage_eight(self) -> None:
        self.path.write_bytes(encode(decoded(5)))
        record = hook.Replay(self.path)
        self.assertEqual(record.label, "SakuyaA Phantasm")
        self.assertEqual(list(record.stages), [8])
        self.assertEqual(record.start(), "7,5,4")

    def test_bad_checksum(self) -> None:
        data = bytearray(self.path.read_bytes())
        data[-1] ^= 1
        self.path.write_bytes(data)
        with self.assertRaisesRegex(ValueError, "checksum"):
            hook.Replay(self.path)

    def test_truncated_file(self) -> None:
        for length in (0, 4, 0x53, 0x54, 100):
            self.path.write_bytes(encode(decoded())[:length])
            with self.subTest(length=length), self.assertRaises(ValueError):
                hook.Replay(self.path)

    def test_invalid_structure(self) -> None:
        for offset, value in [(0x56, 6), (0x57, 6), (0x95, 1), (0x1c, 0), (0x38, 1)]:
            data = decoded()
            data[offset] = value
            self.path.write_bytes(encode(data))
            with self.subTest(offset=offset), self.assertRaises(ValueError):
                hook.Replay(self.path)

    def test_hook_is_published_only_after_loading_finishes(self) -> None:
        entered, release = Event(), Event()
        loaded = ModuleType("replay_hook")

        def load(module: ModuleType) -> None:
            entered.set()
            if not release.wait(5):
                raise TimeoutError("test hook was not released")
            setattr(module, "ready", True)

        spec = SimpleNamespace(loader=SimpleNamespace(exec_module=load))
        with patch.object(replay, "_hook", None), \
                patch.object(replay.importlib.util, "spec_from_file_location", return_value=spec), \
                patch.object(replay.importlib.util, "module_from_spec", return_value=loaded):
            with ThreadPoolExecutor(1) as pool:
                future = pool.submit(replay.hook)
                try:
                    self.assertTrue(entered.wait(5))
                    self.assertIsNone(replay._hook)
                finally:
                    release.set()
                self.assertIs(future.result(5), loaded)
                self.assertTrue(getattr(loaded, "ready"))

    def test_config(self) -> None:
        cfg = hook.config_bytes()
        self.assertEqual(len(cfg), 0x38)
        self.assertEqual(struct.unpack_from("<I", cfg, 0x14)[0], 0x70002)
        self.assertEqual(cfg[0x1f:0x21], bytes(2))  # silent
        self.assertEqual(cfg[0x22:0x24], b"\x01\x00")  # windowed, no skip
        self.assertEqual(cfg[0x25], 0)

    def test_setup_preserves_users_files(self) -> None:
        root = self.path.parent
        original = root / "build/th07_100b.exe"
        original.parent.mkdir()
        original.write_bytes(struct.pack("<3I", 1, 2, 3))
        source = root / "user"
        source.mkdir()
        (source / "harness.rpy").write_bytes(b"user replay")
        (source / "score.dat").write_bytes(b"user scores")
        (source / "th07.cfg").write_bytes(b"user config")
        game = root / "game"
        game.mkdir()
        (game / "replay").symlink_to(source, target_is_directory=True)
        for name in ("score.dat", "th07.cfg"):
            (game / name).symlink_to(source / name)
        with patch.object(hook, "ROOT", root):
            env = hook.setup(str(game), hook.Replay(self.path))
        self.assertEqual((source / "harness.rpy").read_bytes(), b"user replay")
        self.assertEqual((source / "th07.cfg").read_bytes(), b"user config")
        (game / "score.dat").write_bytes(b"run scores")
        self.assertEqual((source / "score.dat").read_bytes(), b"user scores")
        self.assertEqual((game / "replay/harness.rpy").read_bytes(), self.path.read_bytes())
        self.assertEqual(env["TRACE_EXE_CHECKSUM"], "3")
        self.assertEqual(env["TRACE_EXE_SIZE"], "12")

    def test_complete_run(self) -> None:
        with patch.object(replay, "hook", return_value=hook):
            result = replay.validate(hook.Replay(self.path), trace())
        self.assertTrue(result["passed"], result["failures"])

    def test_final_transition_may_consume_last_recorded_input(self) -> None:
        record = hook.Replay(self.path)
        with patch.object(replay, "hook", return_value=hook):
            result = replay.validate(record, trace(frames=4, end_entry=5))
        self.assertTrue(result["passed"], result["failures"])
        with patch.object(replay, "hook", return_value=hook):
            result = replay.validate(record, trace(frames=5, end_entry=6))
        self.assertFalse(result["passed"], "playing a terminal marker must fail")

    def test_marker_input_is_checked_but_uninitialized_events_are_ignored(self) -> None:
        data = decoded()
        for offset in (0xe8 + 0x2c, len(data) - 2 - 8, len(data) - 2 - 4):
            broken = data.copy()
            struct.pack_into("<H", broken, offset, 1)
            self.path.write_bytes(encode(broken))
            with self.subTest(offset=offset), self.assertRaisesRegex(ValueError, "empty input"):
                hook.Replay(self.path)
        for index in (0, 4, 5):
            struct.pack_into("<H", data, 0xe8 + 0x2c + 4 * index + 2, 0xcdcd)
        self.path.write_bytes(encode(data))
        self.assertEqual(hook.Replay(self.path).stages[1]["frame_counts"], [3, 4])

    def test_incomplete_runs_fail(self) -> None:
        for bad in (trace(score=122), trace(frames=2), trace(end_entry=3), trace(title=False)):
            with patch.object(replay, "hook", return_value=hook):
                result = replay.validate(hook.Replay(self.path), bad)
            self.assertFalse(result["passed"], result)

    def test_storage_symbol_aliases_preserve_original_and_older_maps(self) -> None:
        definitions = SimpleNamespace(FUNCTIONS={}, GLOBALS={"Player": ("original", "storage")},
                                      SIZED=[])
        with patch.object(replay, "hook", return_value=definitions):
            for symbol in ("original", "storage"):
                with self.subTest(symbol=symbol):
                    self.assertEqual(replay.resolve({symbol: (0x4BDAD8, None)}),
                                     "Player=0x4bdad8")
            with self.assertRaisesRegex(SystemExit, "original or storage"):
                replay.resolve({})

    def test_original_and_standalone_symbols_resolve(self) -> None:
        original = replay.resolve(replay.symbols_csv())
        self.assertIn("PlayOnUpdate=0x442ee0", original)
        mapfile = ROOT / "build/th07_exe.map"
        if mapfile.exists():
            self.assertIn("PlayOnUpdate=", replay.resolve(replay.map_symbols(mapfile)))

    def test_binary_hook_sites(self) -> None:
        # Check the actual instructions each runtime patch expects, in both
        # executables. No assumptions about rebuilt addresses.
        import compare_exe

        for spec in ("original", "standalone"):
            _, path, symbols = replay.executable(spec)
            if not Path(path).exists():
                continue
            image = compare_exe.PE(path)
            functions = {name: symbols[symbol] for name, symbol in hook.FUNCTIONS.items()}

            def code(name: str) -> tuple[int, bytes]:
                address, size = functions[name]
                return address, image.read(address, size)

            def immediates(container: str, target: str) -> int:
                _, data = code(container)
                value = struct.pack("<I", functions[target][0])
                return sum(data[i:i + 4] == value for i in range(len(data) - 3))

            def calls(container: str, target: str) -> int:
                address, data = code(container)
                return sum(data[i] in (0xe8, 0xe9) and
                           (address + i + 5 + struct.unpack_from("<i", data, i + 1)[0]) & 0xffffffff
                           == functions[target][0] for i in range(len(data) - 4))

            with self.subTest(executable=spec):
                self.assertEqual(calls("RunFrame", "RunCalcChain"), 1)
                self.assertEqual(immediates("SetUpTitleScene", "MenuOnUpdate"), 1)
                for callback in ("PlayOnUpdate", "PlayOnUpdate2", "PlayAddedCallback"):
                    self.assertEqual(immediates("RegisterChain", callback), 1)
                self.assertEqual(calls("RegisterChain", "PlayAddedCallback"), 1)
                self.assertEqual(immediates("RegisterChain", "DeletedCallback"), 2)

    def test_compare_checks_every_record(self) -> None:
        self.assertTrue(replay.compare(trace(), trace())["identical"])
        self.assertFalse(replay.compare(trace(), trace(score=122))["identical"])
        self.assertFalse(replay.compare(trace(), trace(title=False))["identical"])


class LzssTest(unittest.TestCase):
    def test_literals(self) -> None:
        data = bytes(range(256))
        self.assertEqual(hook.decompress(literals(data), len(data)), data)

    def test_overlapping_copy(self) -> None:
        # literal A at ring position 1, followed by a 6-byte copy from 1.
        bits = "1" + f"{65:08b}" + "0" + f"{1:013b}" + f"{3:04b}" + "0" * 14
        bits += "0" * (-len(bits) % 8)
        packed = int(bits, 2).to_bytes(len(bits) // 8, "big")
        self.assertEqual(hook.decompress(packed, 7), b"AAAAAAA")

    def test_zero_tail_omitted_by_compressor(self) -> None:
        self.assertEqual(hook.decompress(literals(b"ABC")[:-1], 3), b"ABC")

    def test_bounds_and_truncation(self) -> None:
        for packed, size in [(b"", 1), (literals(b"ABC"), 2), (literals(b"ABC"), 4), (b"", -1)]:
            with self.subTest(size=size), self.assertRaises(ValueError):
                hook.decompress(packed, size)

    def test_bundled_replays(self) -> None:
        archive = ROOT / "orig/th07_100b/th07.dat"
        if not archive.exists():
            self.skipTest("requires the user's game archive")
        with tempfile.TemporaryDirectory() as directory:
            paths = extract(archive, Path(directory))
            self.assertEqual(len(paths), 3)
            records = [hook.Replay(path) for path in paths]
        self.assertEqual([list(r.stages) for r in records], [[4], [5], [3]])
        self.assertEqual([next(iter(r.stages.values()))["frames"] for r in records], [13451, 11443, 7689])
        self.assertEqual([r.score for r in records], [81032760, 61013780, 26555020])


if __name__ == "__main__":
    unittest.main()
