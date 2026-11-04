"""th07's T7RP format and setup for the shared replay parity harness."""
from __future__ import annotations

import os
from pathlib import Path
import struct
from collections.abc import Callable, Iterator
from typing import TypedDict

ROOT = Path(__file__).resolve().parents[1]
TRACER = ["scripts/replay_trace.cpp"]
WR_TABLE = "PCB_table"
FUNCTIONS = {
    "RunFrame": "?RunFrame@GameWindow@@QAEHXZ",
    "RunCalcChain": "?RunCalcChain@Chain@@QAEHXZ",
    "SetUpTitleScene": "?SetUpTitleScene@@YIHH@Z",
    "MenuOnUpdate": "?OnUpdate@MainMenu@@SIHPAV1@@Z",
    "RegisterChain": "?RegisterChain@ReplayManager@@SIHHPAD@Z",
    "PlayOnUpdate": "?PlayOnUpdate@ReplayManager@@SIHPAV1@@Z",
    "PlayOnUpdate2": "?PlayOnUpdate2@ReplayManager@@SIHPAV1@@Z",
    "PlayAddedCallback": "?PlayAddedCallback@ReplayManager@@SIHPAV1@@Z",
    "DeletedCallback": "?DeletedCallback@ReplayManager@@SIHPAV1@@Z",
}
SIZED = ["RunFrame", "SetUpTitleScene", "RegisterChain"]
GLOBALS = {
    "GameManager": "?g_GameManager@@3VGameManager@@A",
    "App": "?g_App@@3VApp@@A",
    "Random": "?g_Rng@@3VRng@@A",
    "Buttons": "?g_GameInputs@@3GA",
    "Player": ("?g_Player@@3VPlayer@@A", "?g_Player@@3UPlayerStorage@@A"),
    "BulletManager": "?g_BulletManager@@3VBulletManager@@A",
    "EnemyManager": "?g_EnemyManager@@3VEnemyManager@@A",
    "ItemManager": "?g_ItemManager@@3VItemManager@@A",
}
SHOTS = ["ReimuA", "ReimuB", "MarisaA", "MarisaB", "SakuyaA", "SakuyaB"]
DIFFICULTIES = ["Easy", "Normal", "Hard", "Lunatic", "Extra", "Phantasm"]
CONFIG = "th07.cfg"
REPLAY_PATH = "replay/harness.rpy"


class Stage(TypedDict):
    score: int  # native game units: ten points
    frames: int
    frame_counts: list[int]
    entries: int
    seed: int
    lives: int
    bombs: int
    power: int


def decompress(data: bytes | bytearray, size: int) -> bytes:
    """The game's MSB-first LZSS (src/lzss.cpp), with bounded output."""
    if not 0 <= size <= 32 * 1024 * 1024:
        raise ValueError("invalid LZSS output size")
    # The compressor omits its partially filled last byte. The game
    # supplies zero bytes at EOF, completing the zero end marker.
    data = bytes(data) + bytes(2)
    bit = 0

    def read(count: int) -> int:
        nonlocal bit
        if bit + count > len(data) * 8:
            raise ValueError("truncated LZSS stream")
        value = 0
        for _ in range(count):
            value = (value << 1) | ((data[bit // 8] >> (7 - bit % 8)) & 1)
            bit += 1
        return value

    window = bytearray(0x2000)
    pos = 1
    out = bytearray()
    while True:
        if read(1):
            chunk = [read(8)]
        else:
            source = read(13)
            if source == 0:
                break
            count = read(4) + 3
            # Copies may overlap their source in the ring buffer.
            chunk = []
            for i in range(count):
                value = window[(source + i) & 0x1fff]
                chunk.append(value)
                window[(pos + i) & 0x1fff] = value
        if len(out) + len(chunk) > size:
            raise ValueError("LZSS output exceeds declared size")
        for value in chunk:
            out.append(value)
            window[pos] = value
            pos = (pos + 1) & 0x1fff
    if len(out) != size:
        raise ValueError("LZSS output does not match declared size")
    return bytes(out)


class Replay:
    def __init__(self, path: str | os.PathLike[str]) -> None:
        self.path = Path(path)
        data = bytearray(self.path.read_bytes())
        if len(data) < 0x54 or data[:4] != b"T7RP":
            raise ValueError(f"{path}: not a th07 replay")
        self.version = struct.unpack_from("<H", data, 4)[0]
        if self.version != 0x1100:
            raise ValueError(f"{path}: unsupported version {self.version:#x}")
        key = data[0xd]
        for i in range(0x10, len(data)):
            data[i] = (data[i] - key) & 255
            key = (key + 7) & 255
        if (0x3f000318 + sum(data[0xd:])) & 0xffffffff != struct.unpack_from("<I", data, 8)[0]:
            raise ValueError(f"{path}: bad checksum")
        total, packed, unpacked = struct.unpack_from("<III", data, 0x10)
        if packed != len(data) - 0x54 or total != unpacked + 0x54 or total < 0xe8:
            raise ValueError(f"{path}: inconsistent replay sizes")
        data = data[:0x54] + decompress(data[0x54:], unpacked)
        self.shot, self.difficulty = data[0x56:0x58]
        if self.shot >= len(SHOTS) or self.difficulty >= len(DIFFICULTIES):
            raise ValueError(f"{path}: invalid shot or difficulty")
        self.date = data[0x58:0x5e].split(b"\0")[0].decode("ascii", "replace")
        self.name = data[0x5e:0x6a].split(b"\0")[0].decode("shift_jis", "replace").strip()
        self.score = struct.unpack_from("<I", data, 0x6c)[0] * 10
        self.slowdown = struct.unpack_from("<f", data, 0xcc)[0]
        self.config = bytes(data[0x70:0xa8])
        if self.config[0x25]:
            raise ValueError(f"{path}: score-disabled replay cannot play inputs")
        offsets = struct.unpack_from("<7I", data, 0x1c)
        extras = struct.unpack_from("<7I", data, 0x38)
        boundaries = sorted({*offsets, *extras, len(data)} - {0})
        self.stages: dict[int, Stage] = {}
        for idx, off in enumerate(offsets):
            if not off:
                continue
            # SaveReplay writes ALL stages, then ALL FPS streams. In
            # particular, the last stage ends at the FIRST FPS stream,
            # which may belong to an earlier stage. stageSizes in the
            # game's loader overestimates this; playback never uses it.
            end = next((value for value in boundaries if value > off), len(data))
            extra_end = next((value for value in boundaries if value > extras[idx]), len(data))
            if not 0xe8 <= off < off + 0x2c + 4 <= end <= extras[idx] < extra_end <= len(data):
                raise ValueError(f"{path}: invalid stage {idx + 1} offsets")
            if (end - off - 0x2c) % 4:
                raise ValueError(f"{path}: misaligned stage inputs")
            # Recording starts at frames+1. DeletedCallback appends one
            # empty frame at every stage; SaveReplay appends a SECOND
            # empty frame to the final stage when the file is saved.
            # Events in those slots are uninitialized (often 0xcdcd),
            # so they cannot be used as sentinels.
            entries = (end - off - 0x2c) // 4
            final = off == max(offsets)
            padding = 3 if final else 2
            if entries <= padding or struct.unpack_from("<H", data, off + 0x2c)[0] != 0:
                raise ValueError(f"{path}: missing initial empty input or gameplay frames")
            for marker in range(entries - (2 if final else 1), entries):
                if struct.unpack_from("<H", data, off + 0x2c + 4 * marker)[0] != 0:
                    raise ValueError(f"{path}: missing terminal empty input")
            frames = entries - padding
            # Recording writes AFTER the GUI, playback reads BEFORE it.
            # Stock runs end either before or after consuming the final
            # recorded input; the file has no explicit playback count.
            # Both endpoints leave the two true markers unplayed. Earlier
            # stages have only one marker and require the single endpoint.
            frame_counts = [frames, frames + 1] if final else [frames]
            seed, power, lives, bombs = struct.unpack_from("<HBBB", data, off + 0x20)
            stage = 8 if self.difficulty == 5 else idx + 1
            self.stages[stage] = Stage(score=struct.unpack_from("<I", data, off)[0],
                                       frames=frames, frame_counts=frame_counts,
                                       entries=entries, seed=seed,
                                       lives=lives, bombs=bombs, power=power)
        if not self.stages:
            raise ValueError(f"{path}: no stages")

    @property
    def label(self) -> str:
        return f"{SHOTS[self.shot]} {DIFFICULTIES[self.difficulty]}"

    def describe(self) -> Iterator[str]:
        yield f"{self.name} on {self.date}, score {self.score}, slowdown {self.slowdown:.2f}%"
        for stage, record in self.stages.items():
            counts = " or ".join(map(str, record["frame_counts"]))
            yield f"stage {stage}: ends with {record['score'] * 10} after {counts} replay calculations"

    def start(self) -> str:
        return f"{min(self.stages) - 1},{self.difficulty},{self.shot}"


def config_bytes() -> bytes:
    cfg = bytearray(0x38)
    cfg[:0x12] = bytes.fromhex("0000010002000400ffffffffffffffff0300")
    struct.pack_into("<I2h", cfg, 0x14, 0x70002, 600, 600)
    cfg[0x1c:0x24] = bytes([2, 3, 0, 0, 0, 1, 1, 0])
    cfg[0x24] = 2
    struct.pack_into("<I", cfg, 0x34, 1)
    return bytes(cfg)


def setup(game_dir: str, replay: Replay) -> dict[str, str]:
    # The shared runner links DAT files and directories. score.dat is
    # writable, and replay/ may contain the user's own replays.
    score = Path(game_dir, "score.dat")
    if score.is_symlink():
        saved = score.read_bytes()
        score.unlink()
        score.write_bytes(saved)
    directory = Path(game_dir, "replay")
    if directory.is_symlink():
        directory.unlink()
    directory.mkdir(exist_ok=True)
    for name, data in ((CONFIG, config_bytes()), (REPLAY_PATH, replay.path.read_bytes())):
        path = Path(game_dir, name)
        path.unlink(missing_ok=True)
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)
    # The imported tracer changes the executable's checksum. Keep the
    # original identity for th07's periodic version-file check.
    original = ROOT / "build/th07_100b.exe"
    data = original.read_bytes()
    checksum = sum(struct.unpack_from(f"<{len(data) // 4 - 1}I", data)) & 0xffffffff
    return dict(TRACE_REPLAY=REPLAY_PATH, TRACE_START=replay.start(),
                TRACE_EXE_SIZE=str(len(data)), TRACE_EXE_CHECKSUM=str(checksum))


def stage_failures(stage: int, recorded: Stage, last: tuple[int, ...], end: tuple[int, ...],
                   get: Callable[[tuple[int, ...], str], int]) -> list[str]:
    failures = []
    consumed = get(last, "entry")
    if consumed != get(last, "replayFrame") or consumed not in recorded["frame_counts"]:
        failures.append("last calculation did not consume the recorded inputs")
    if get(end, "entry") != consumed + 1:
        failures.append("stage did not end on the recorded empty frame")
    return failures
