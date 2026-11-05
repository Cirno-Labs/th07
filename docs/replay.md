# Replay parity

The shared runner from th06 is available through `just replay`. th07's
format and symbols live in `scripts/replay_hook.py`; its injected tracer
is `scripts/replay_trace.cpp`. The runtime, null renderer, Wine/Xvfb
isolation, validation, comparison and evidence format remain in
`tools/replay/`.

```sh
just exe
python3 tools/replay/replay.py fetch      # requires network access
just replay                             # all downloaded manifest entries
just replay replays/example.rpy          # a particular recording
python3 tools/replay/replay.py info replays/example.rpy
python3 -m unittest scripts.test_replay
python3 -m unittest discover -s tools/replay
```

Without a downloaded corpus, extract the three recordings shipped in
your own game archive, into ignored build output:

```sh
python3 scripts/replay_fixtures.py
just replay build/replay/fixtures/*.rpy
```

These are truncated single-stage demo recordings. Parsing them is useful
for format checks, but playing them as ordinary replays can run past their
recorded inputs; the harness correctly rejects that as incomplete. They
provide no full-route coverage. Do not commit extracted game data. `fetch` uses the PCB world
record table and retains source URLs, players, dates and hashes in the
manifest, as the shared harness does for th06.

## What is compared

By default every recording runs in the original twice, to check
repeatability, and in the standalone `build/th07.exe`. Each uses its own
symbol table: `config/symbols.csv` for the original and the link map for
the rebuild. The tracer validates the number of hook sites it replaces.
Hooked DLL builds are not supported.

The tracer starts playback with the replay menu's settings after 60 title
updates, wraps th07's fastcall replay callbacks, and records every replay
calculation, stage boundary, natural playback deletion and return to the
title. It counts main-loop passes separately from dialogue fast-forwards.
The title is th07 menu state 0/substate 1 (th06's menu-state numbers do
not apply). The tracer reads event words from the recorded input stream:
ReplayManager::frameEvents is never initialized during playback.
It exits only after the playback deletion and another 60 updates with the
title active; crashes, limits and timeouts fail.

A trace includes score and shown score, lives, bombs, power, graze, point
items, rank, deaths, spell cards, cherry counters, RNG state, input and
recorded replay events, player position, entity counts and hashes of active
bullets, lasers, enemies, items and player shots. Float values are stored
as their bits. Trace scores and stage validation use the game's units of
ten points, avoiding overflow of the 32-bit trace fields. `info` displays
actual points. After returning to the title, newly allocated GameStats
contains uninitialized heap data; title records leave its fields zero.

The format parser decrypts T7RP, verifies its checksum and sizes, and
bounds LZSS decompression. SaveReplay writes all stages before all FPS
streams. Recording leaves an initial empty input, DeletedCallback adds
an empty frame at each stage end, and saving adds a second empty frame
to the final stage. Those frames' event words are uninitialized; the
parser counts physical entries rather than looking for event sentinels.
Phantasm occupies replay slot 7 but runs as game stage 8.

A passing run must reach the format's complete-input endpoint, finish each
stage with its recorded score, play the stages in order and return
naturally to the title. An identical prefix or a forced early exit is
not a pass.

The format stores recording inputs, not a separate playback frame count.
Recording writes inputs after the GUI callback, whereas playback reads
before it. For a final stage with N recorded inputs, stock runs can end
after N or N+1 playback calculations, excluding or including the last
recorded input. Both observed endpoints leave the two true terminal
markers unplayed. The parser verifies the initial input and both marker
inputs are zero (their event words are uninitialized), and exposes only
those two endpoints through the shared validator's optional frame_counts.
The exact reason some recordings consume the surplus input is unproven.
Earlier stages require their single endpoint: an extra calculation there
makes EndStage overwrite the next stage's header. The tracer also checks
entry == replayFrame and end.entry == last.entry + 1; complete trace
comparison still compares every field of every calculation.

`build/replay/<case>/evidence.json` contains the run checks, hashes and
complete trace comparisons. `scripts/audit_replays.py` revalidates saved
runs with the current parser,
verifies the recorded hashes of the replay, executable, instrumented
executable, tracer and trace, and compares complete traces. It writes a
separate ignored audit report without changing the raw run assessments:

```sh
python3 scripts/audit_replays.py          # original, original+, standalone
python3 scripts/audit_replays.py --labels original original+ matching_exact --out build/replay/matching-exact-audit.json
```

## Instrumentation and limits

The runner modifies a copy of each executable's import table. The
tracer installs the shared clock and null renderer, then makes the
performance counter use that same clock with a 1000 Hz frequency so
th07's main-loop pacing can accelerate too. `--speed 1 --render` uses
real-time pacing and drawing; `--speed 0` is the default accelerated run.
Audio is disabled. Rendering and audio output are not compared; game
draw callbacks still run.

Adding an import changes the executable checksum. Before the first
update, the tracer restores App's executable size/checksum fields to the
verified original's identity. This preserves the periodic version-file
check while permitting instrumented/rebuilt executables to run. The
replay's embedded version and checksum are still checked by the game.
The run records the actual executable and instrumented file hashes.

The run's config is windowed, with no audio or frame skip. Writable
`score.dat`, configuration and replay files are private copies; setup
removes the shared runner's symlinks for these paths.

## Validation status

The final standalone executable is byte-identical to the original:
**650,752 bytes**, SHA-1 `80cbf99382c81ea0a9b7b85c7f9e059a75a52725`.
A clean original-toolchain rebuild reproduces it, and `just check-exe`
enforces whole-file equality. See [executable parity](exe-parity.md) for
its documented compiler layout adapters.

All **36 complete trace files are byte-identical** to both original runs:
5,212,055 records and 239,754,530 fields per original-versus-rebuild
comparison. This includes the natural return to the title and the
original's failed recording playbacks. The final saved-artifact audit is
`build/replay/matching-exact-audit.json`; all executable hashes are the
original hash, and every replay, executable, patched executable, tracer
and trace hash verifies.

**26 of 36 recordings pass complete-route validation**, covering
3,395,051 gameplay calculations in each executable. The remaining 10
are excluded: all six Lunatic recordings, Easy ReimuB, and Hard ReimuB,
MarisaA and SakuyaB. Their original runs fail the same input/score checks.
The audit deliberately exits nonzero for those coverage failures while
reporting 36 identical trace comparisons.

| Difficulty | Complete recordings passing and matching |
| --- | ---: |
| Easy | 5 / 6 |
| Normal | 6 / 6 |
| Hard | 3 / 6 |
| Lunatic | 0 / 6 |
| Extra | 6 / 6 |
| Phantasm | 6 / 6 |

For Lunatic ReimuA, enabling actual rendering reproduced the null-renderer
trace exactly: 30,661 records and 1,410,406 values, including the extra
stage-1 calculation and natural death in stage 2. This rules out skipped
rasterization as that recording's failure cause; other harness/platform
effects remain unproven. It remains excluded from route coverage.

The Python suites pass **31 tests**: 23 project tests and eight shared
harness tests. They cover real bundled recordings, checksums, bounded
multi-stage/Phantasm layouts, complete-input markers, corrupt/incomplete
traces, private writable files, actual binary hook sites, concurrent hook
loading, and original/older/storage-global symbol spellings. The tracer
is compiled with the original MSVC toolchain.

Earlier audits remain under `build/replay/` as separate checkpoints:
`gm-audit.json`, `final-audit.json`, `matching-3208-audit.json` and
`matching-2062-audit.json`. Old raw assessments are unchanged; each audit
records the validator hashes used to recheck its saved artifacts.

Trace equality and full-route coverage are separate results. An identical
comparison or a parsed replay file alone does not establish route coverage.
