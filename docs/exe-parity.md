# Executable parity

The canonical standalone build achieves **100% whole-file byte identity**:
**all 650,752 bytes match** retail th07 1.00b, with zero differing bytes.
Both `build/th07.exe` and `build/th07_100b.exe` have SHA-1
`80cbf99382c81ea0a9b7b85c7f9e059a75a52725`.

```sh
just check-exe             # builds the exe, then requires exact equality
just compare              # ordinary-object function comparison
just compare-exe          # detailed headers/sections/symbol report
```

`scripts/check_exe.py` compares every byte and the file lengths. It exits
nonzero on any mismatch, including headers, padding, tables and metadata;
it does not mask addresses. Ordinary comparison passes **702/702
functions, 381,300 bytes**. All **694 standalone game functions** sit at
their original addresses and match **raw**, as do the complete D3DX and
CRT regions. Headers, Rich metadata, initialized data and resources also
match exactly. Replay validation and its coverage limits are recorded in
[replay parity](replay.md).

The ignored `build/parity_final/proof.json` records the current source and
toolchain hashes, clean-build checks and final replay-audit fingerprint.

This is executable identity, **not a claim to have recovered the original
source**. The final build uses the original compiler/linker and documented
source/compiler layout aids. No generated instruction or data bytes are
patched or inserted from the game executable. The existing resource reuse
and link-timestamp copying in the build remain unchanged.

## Reproducibility checks

A cold rebuild of **all 37 ordinary objects and all 37 `/GL` inputs**,
through the normal canonical source paths with generated flag caches
cleared, reproduces the exact hash above. Its ordinary comparison passes
702/702, and its standalone comparison confirms all 694 game functions
match raw at their original addresses. All 76 mapped globals retain their
original addresses, including the storage symbol aliases. The saved reports
are `build/parity_final/clean_rebuild.log`, `functions.txt` and
`executable.txt`.

All **30 Python regression tests pass**: 23 project tests and seven shared
harness tests. Independent cold/cache-hit CRT preparation checks confirm
the 715-member order, every member's bytes, the intact throw tail, and an
unchanged installed toolchain archive. The exact executable's replay audit
is documented separately in [replay parity](replay.md).

## What the final build retains

The matching game logic is accompanied by explicit reconstruction aids:

- **Scalar and string pools.** `UnusedScalarOrder` references the same
  192 live scalar constants in compiler-emission order, reproducing their
  808-byte pool with normal double alignment. An unused `selectany`
  pointer table in ascii.cpp seeds the four existing late ASCII strings.
  The helper code and pointer table are removed by `/OPT:REF`; their
  existing constants retain the original addresses.
- **Identical COMDAT folding (ICF).** Eight marked, discarded integer
  functions control the legacy linker's unstable tie ordering. Three
  discarded allocation paths control exception-section ordering. An App
  callback body clone, retained through an explicit linker reference,
  folds with the original callback; the original body is the kept copy.
  These reproduce the original code/EH layout without extra retained
  helper bodies or exception payload. A real REF/NOREF trace and a sort
  model guided the final 14 retained-code fold constraints. Unused-only
  classes are excluded from those constraints.
- **Static-construction stack layout.** `AsciiTextStorage` and
  `PlayerStorage` are source-only derived storage adapters. Their fully
  inlined constructors contain respectively **20 and 4 bytes of unused
  stack locals**, never read or written. They reproduce the original
  initializer reservations `0x54` and `0x64` while leaving the core game
  constructor instructions and object storage sizes unchanged. These
  adapters are intentional layout aids, not claimed original source.
  The symbol configuration and replay-map resolver accept both original
  and storage global names. Stage's `0x44` reservation is recovered naturally
  by spelling its existing camera copies as `memcpy`, with identical core
  constructor bytes and relocations.
- **Real CRT archive contributions.** `scripts/crt_archives.py` uses the
  installed MSVC `lib.exe` to produce an ordered common archive and a
  late archive containing the actual `throw.obj`. It validates all
  **715 retained common members in their original order and with their
  original bytes**, plus the intact tail member, and checks that the
  installed archive was not modified. Inputs and generated archives are
  hash-checked for caching. A discarded source reference pulls the real
  CRT filter pointer into its original slot at `0x49ea6c`; it replaces
  an earlier guessed contributor, rather than adding a fabricated
  pointer or padding. Rich counts, ordering and checksum match exactly.

Other retained corrections include explicit zero initialization and
ordering of static storage, 37 real `/GL` ownership units, and the
Windows SDK compile definitions `/DWIN32` for dsutil.cpp and `/D_WINDOWS`
for midi.cpp. The two unused Item chain objects retain their addresses
while declaration order restores their construction/destruction order.

The final byte check also exposed a functional key-configuration error:
row 3 must assign message skip, followed by pause and the four directions.
Correcting only the switch labels restores its eight mismatching jump-table
bytes and agrees with the existing displayed/help row order. Function-only
matching had not caught this generated table.

Use the configured toolchain and canonical build paths when validating a
change. Isolated source paths produced different sound/MIDI vtable emission
in experiments, even with matching function shapes; only the normal build
and the strict whole-file gate establish the final result.

## Earlier checkpoints and experiments

The measurements and proposed next steps below are historical. They explain
how the matching build was reached; they are not remaining work or current
failure counts.

### Source changes that improved the build

MSVC's allocation order for constructed globals depends on their
identifiers. Small programs with the same constructor/destructor
properties reproduced each game's object-group ordering. Trying clear
names for the chain elements and other globals found names that put the
existing objects in the original order. Fresh full-game compilation and
linking confirmed each resulting address; the constructors and update/
draw functions retain their matching instructions.

This is an empirical correction of our names, not a claim to have
recovered the original identifiers or the backend's complete ordering
algorithm. No padding globals or pointer redirections were introduced.

| Object group | Result |
| --- | --- |
| Logger and chain | `g_Logger` at `0x624210`, `g_Chain` at `0x626218` |
| Game manager | Manager `0x626270`, update `0x62f8b4`, render `0x62f8d4` |
| GUI | Draw and update chain elements in the original order |
| Bullets | Render chain, manager, update chain in the original order |
| Enemies | Render chain, manager, update chain, second render chain in the original order |
| Effects | Manager, update chain, draw chain in the original order |
| Enemy scripts | Scripts `0x1347938`, unused chain element `0x1347ac0` |
| Stage | Draw chain, `g_StageManager`, second draw chain, update chain in the original order |
| ASCII | Render chain, text object, update chain, second render chain in the original order |
| Sprite vertices | `g_QuadVertices` at `0x4ba018`, diffuse XYZ vertices at `0x4ba078` |

All **76 mapped `.data`/`.bss` globals now sit at their original
addresses**, up from 51. Twenty-three other entries in symbols.csv have
no public map symbol, largely function-local statics, and still need
separate examination.

The File modes table was also split into `src/filemodes.cpp`, a data-only
translation unit placed immediately after `file.obj` by
`scripts/build_hook.py`. This reproduces the original strings and File
vtables at `0x49523c`–`0x49528c` exactly and preserves the mode-pointer
table's original address. It adds one `/GL` object: 33 versus the
original's 37, rather than the earlier 32. The game reads the same
`"r"`, `"w"` and `"a"` modes as before.

Measured isolated-build progression:

| Candidate | Differing bytes |
| --- | ---: |
| Starting build | 6,278 |
| Game manager allocation | 4,961 |
| Additional constructed-global groups | 4,088 |
| Effects plus File mode split | 3,961 |
| Stage and sprite vertices | 3,809 |
| Unused enemy-script chain | **3,766** |

The final ordinary and standalone builds were regenerated after the
source changes. The final isolated executable and canonical
`build/th07.exe` have identical hashes.

### Earlier 3,766-byte breakdown

The files have identical length and section raw offsets/sizes. Counts
below include all unequal byte pairs at each file offset, including
padding:

| File region | Before | Now |
| --- | ---: | ---: |
| Headers | 320 | 320 |
| `.text` | 5,128 | 2,659 |
| `.rdata` | 720 | 680 |
| `.data`, initialized bytes on disk | 110 | 107 |
| `.rsrc` | 0 | 0 |
| **Whole file** | **6,278** | **3,766** |

The `.text` differences can be partitioned by original virtual address.
Intervals include their start and exclude their end:

| Original address interval | Contents | Differing bytes |
| --- | --- | ---: |
| `0x401000`–`0x46004b` | Game functions | 749 |
| `0x46004b`–`0x461aa2` | Tables and import thunks | 0 |
| `0x461aa2`–`0x47d136` | Original D3DX8 region | 95 |
| `0x47d136`–`0x48c3a0` | Original CRT before EH helpers | 6 |
| `0x48c3a0`–`0x48c600` | EH helpers | 219 |
| `0x48c600`–`0x48cd38` | Static initializers and text tail | 1,590 |

These are address-region counts rather than a semantic attribution of
every byte to a library. `.text` virtual size remains `0x8bd28` versus
`0x8bd38`, while both raw sections occupy `0x8be00` bytes. The import
directory, IAT, imported functions/order and resources match entirely.

| File | SHA-1 |
| --- | --- |
| Earlier 3,766-byte candidate | `f78432c8cfc8b7c3510652bbfe050f7f0dd6b690` |
| Starting standalone build | `8c26872840e6cb7efdcd303d5c8ccbc27ab6e072` |
| Original `build/th07_100b.exe` | `80cbf99382c81ea0a9b7b85c7f9e059a75a52725` |

### Experiments identified at 3,766 bytes

1. **LTCG generation order and EH helpers.** Most remaining text bytes
   are in the initializer tail, shifted by the `0x10` EH-layout gap.
   Ending::RegisterChain's EH group and the last initializer groups have
   different emission order. The float pool also remains coupled to
   generation order. Read the existing models and sessions 12–13 in
   `handoff/drafts/exe/notes.md` before changing source.
2. **Function-local statics.** Their public map symbols are absent, and
   their addresses still differ: RunFrame's clocks, DrawFps's counters/
   text, CheckSingleInstance's mutex, the MIDI timer counter and joystick
   buttons. Their allocation pass needs its own compiler experiment.
3. **Smaller pools and metadata.** The ASCII strings and two vftable
   pairs still have ordering differences. Rich metadata still has 33
   rather than 37 `/GL` objects, two rather than three alias objects,
   and four rather than three C objects from build 9178. Filling counts
   without preserving code generation and ICF can regress the build.

The earlier `dataref.py` claim of a displaced `g_EnemyScriptInterps`
table was a heuristic false positive: both executables contain the same
eight-pointer table at `0x49f3ec`. That script scans overlapping words,
so its apparent mismatches must be checked against actual instructions.

Other discarded experiments in this session: moving g_FileModes within
file.cpp left its pool unchanged; a named constant-array alternative
regressed the pool; removing `/MT` only from midi.cpp and a timer-class
rename did not change the mismatched vtable order. Making empty ASCII/
Player constructors implicit moved matching functions and regressed the
build; adding inline to a constructor declaration changed nothing.

At this checkpoint executable identity had not yet been reached. Replay
coverage is reported separately in [replay parity](replay.md); byte identity
and whole-route behavioral validation are separate checks.

#### Legacy SDK dependency probe

The installed SDK provides a concrete candidate explanation for the
missing earlier-callee edge in the float-pool model. Its legacy
`d3dtypes.h`, with `D3D_OVERLOADS`, defines an empty `_D3DVECTOR()`
constructor before `D3DXVECTOR3`; DX8's `d3d8types.h` defines an
aggregate instead. Actual `/Ob0` compilation confirms that the legacy
`D3DXVECTOR3(float,float,float)` calls that earlier base constructor,
while the DX8 form has no callees. Small `/Ob1` vector-construction
functions remain byte-identical.

A full isolated experiment used `/DD3D_OVERLOADS`,
`/DDIRECT3D_VERSION=0x0800`, and `/FId3dtypes.h`. Only **695/702** ordinary
function comparisons passed; seven functions changed. Its standalone
layout regressed. Applying the legacy header only to the first ASCII
object preserved all 694 game-function addresses and masked bytes, but
changed library folds, added `0x10` read-only data and increased whole-
file differences to 69,471. A normalized 179-dword float-pool comparison
improved from an LCS of 134 to 143, so the dependency affects the right
pool but is not a complete solution. Mixed type definitions across
translation units would also need justification. No SDK changes were
retained. Probe sources/objects remain in ignored
`build/parity_small/sdktoy/` and `build/parity-experiments/dx7_*`.

#### Timer helper first-use probe

A bounded follow-up addressed the earlier failed same-object-helper
experiment. A meaningful `ClearFraction()` helper is first used in
Timer's existing constructor/Initialize before Set uses it. A
`SavePrevious()` helper is first used in Timer::NextTick, which is
defined early in ascii.cpp, before Tick uses it. These earlier first
uses differ from adding a helper called only inside Set, where the
helper's node appears after its caller and causes no delay.

The isolated full-game experiment preserves **702/702** ordinary
function comparisons and all **694** game-function addresses/masked
bytes. Its normalized float-pool LCS improves from 134 to 140 of 179
dwords, but changed ICF choices regress whole-file differences to
92,169. Canonical sources were not changed. The copied sources and
objects remain under ignored `build/parity-experiments/timer_helpers/`;
the driver is `build/parity-experiments/timer_helpers.py`. Testing this
route further requires a fresh CIL/ICF trace and retuning the existing
dead-function constraints together with the altered generation order.
It is not a verified executable improvement.

#### Explicit zero initialization of function statics

The remaining function-local static addresses are reproduced by explicit
constant-zero initializers, rather than identifier guesses. MSVC puts
these initialized statics ahead of ordinary uninitialized function
statics, in reverse declaration order. DrawFps initializes frames,
fpsText, speedText and measures (in that declaration order), while its
lastCounter remains uninitialized. RunFrame initializes lastCount,
lastFrameTime and frames in that declaration order, and the instance
mutex initializes to NULL. C++ already zero-initializes static storage;
these declarations preserve behavior and change the compiler's allocation
pass.

A fresh isolated full build at build/parity-experiments/locals_verified/
reduces differences from 3,766 to 3,674 and preserves 702/702 ordinary
function comparisons. Every identified tail static now has its original
address, including MIDI and joystick buffers that were not edited.
The .data virtual size remains exact. Name changes and declaration
permutations without explicit initializers did not change the allocation.

#### Rich contributors and DOS-header placement

The remaining Rich count classes have source-level explanations. Splitting
actual renderer, replay, input and file-size globals out of the existing
data-only unit adds four real `/GL` inputs (33 to the original 37), while
preserving their allocation order. The original unit retains the unused
word and constructed archive array; the four new units carry no
constructors, code, duplicate storage or added data.

An unused `selectany` constant pointer to `CLSID_ShellLink` in the early
ASCII unit makes uuid.lib load shguids.obj first. That member supplies
both CLSID_ShellLink and IID_IShellLinkA, eliminating the extra
shobjidl_i.obj and reproducing the original three build-9178 objects.
The unused pointer itself is removed by `/OPT:REF`; the program's code
and data are unchanged. Similarly, a pointer to the legacy C name
`stricmp` in the final data unit pulls the missing third alias object;
its target `_stricmp` implementation is already needed by `strcmpi`.
Pointers to `_mbscpy`, `_mbscat` or `_mbsdup` change library pull timing
and regress the layout, so those were rejected.

Link.exe's Rich builder at `0x457320` explains the apparent extra DOS
padding: the checksum is a shared DOS-stub base `0x884f3421` plus the
sum of `ROL32(compid, count)`. At `0x457444` the linker reserves
`0x20 + 8 * (distinct_products + ((checksum >> 5) % 3))` bytes. Correct
counts therefore also restore the original Rich key and PE-header offset
`0x118`, without custom stubs or header rewriting.

An isolated combined build at `build/parity_locals/combined/` incorporates
these contributor changes and the verified explicit-zero locals. It has
**3,355 differing bytes**: one header byte (the remaining `.text` virtual
size), 2,567 `.text`, 680 `.rdata`, and 107 initialized `.data` bytes.
All Rich entries, their order, counts, key and padding match the original.
The unused pointer COMDATs are absent from its public map.

#### Replay-menu initialized table declarations

`DrawReplayMenu`'s existing static tables belong in source order shot
names, difficulties, stages. The compiler's reversed initialized-static
allocation then puts the three pointer tables at the original addresses,
and the changed first occurrence of shared strings also reproduces the
original Stage/Difficulty string pool. No values or uses change.
An isolated local-static-baseline build improves from 3,674 to 3,554
bytes differing; together with the metadata contributor fixes the
candidate `build/parity_locals/combined_menu/candidate.exe` has **3,235
bytes differing**.

The metadata contributors, menu order and filename-table improvements are now
integrated in canonical source. Player and Ending name their existing filename
storage separately from the pointer tables, allowing the table declarations to
be reversed without disturbing the original string pool. This replaces the
anonymous literals with named arrays; it adds no retained bytes.

That integrated build has **3,208 differing bytes**, SHA-1
`c63cd0e378274ec26be9f0516c349c28801f3821`: one header byte, 2,557 `.text`,
604 `.rdata`, 46 initialized `.data`, and zero `.rsrc` bytes. It is byte-identical
to the isolated combined candidate. Ordinary comparison remains 702/702; all
694 standalone game functions retain their original addresses and match with
address operands masked. The remaining initialized-data differences include
43 bytes across 41 startup-function pointers associated with the EH layout and
three bytes in the four-byte unreferenced gap at `0x49ea6c`; no storage was added to imitate
that gap.

#### Windows SDK compile definitions and vtable emission

The sound and MIDI files need distinct standard Windows SDK definitions:
`// cflags: /DWIN32` in dsutil.cpp and `// cflags: /D_WINDOWS` in midi.cpp.
These restore both sound vtables and both MIDI vtables, including the
corresponding constructor/destructor address operands. They change only
18 executable bytes, all toward the original; function shapes and ordering
remain unchanged. This is an empirical correction of compiler settings,
not proof of the original project's complete settings.

Canonical source paths, repeat compilations and the physical first-line
flag directives reproduce the result. An earlier isolated header-include
probe did not reproduce through the normal build and was rejected. The
combined canonical executable is **3,190 differing bytes**, SHA-1
`505223bd8ff7ef9b751d7fdb0af1637838956f40`: one header byte, 2,549 `.text`,
594 `.rdata`, 46 `.data`, and zero `.rsrc`. Rich metadata matches exactly;
702/702 ordinary comparisons and all 694 standalone addresses/masked
comparisons pass, with 569 standalone functions matching raw.


#### Scalar-constant order and joint ICF constraints

The later **2,054-byte checkpoint** has SHA-1
`b67c861e6c5fe9deb6fdd05e3e4a1cad20a90922`. All 694 game functions stay
at their original addresses, 676 match raw, and all 694 match with address
operands masked. A clean compile of all ordinary objects and all 37 `/GL`
translation units through the canonical paths verified the scalar/ICF
result. The subsequent Stage rebuild described below supplies that checkpoint
hash; ordinary comparison remained **702/702**.

`UnusedScalarOrder` in file_globals.cpp is an explicit synthetic layout
aid, not a recovered original function. It performs arithmetic using the
same 192 typed scalar constants already referenced by live game code.
The legacy compiler creates their COMDATs in reverse statement order;
placing this discarded helper first in CIL generation seeds the existing
constant cache in the original pool order. `/OPT:REF` removes its code.
The live users retain exactly the existing 808 bytes of constants, now
all at their original addresses, with normal eight-byte double alignment.
No original executable bytes or padding are inserted into an object.

The additional CIL contribution also changes the legacy linker's unstable
ICF tie ordering. Eight explicitly marked discarded integer helpers
restore the original kept copies. Their source bodies use ordinary
integer arithmetic; a joint sort model chose positions and sizes, and a
fresh real compiler/linker trace confirmed all 19 kept-copy constraints.
These helpers are layout stand-ins, not claimed original source.

Both complete address regions `0x461aa2`–`0x47d136` (D3DX) and
`0x47d136`–`0x48c3a0` (CRT before the exception helpers) are now byte
identical. All 195 mapped CRT functions match raw at their original
addresses; all 55 placed D3DX object chunks have the original addresses
and masked bytes. `.rdata` has the original virtual size `0xe820`.

| 2,054-byte checkpoint region | Differing bytes |
| --- | ---: |
| Headers | 1 |
| `.text` | 1,839 |
| `.rdata` | 168 |
| Initialized `.data` | 46 |
| `.rsrc` | 0 |
| **Whole file** | **2,054** |

Of the `.text` differences, 30 are in the game region, 219 in exception
helpers, and 1,590 in static initializers and the text tail. The text
virtual size remains 16 bytes shorter than the original. Two static
initializer frame sizes, EH kept-copy/layout ordering, and startup-table
pointers remained unresolved at that checkpoint.

The isolated candidate initially measured 2,062 differences. Compiling
through the normal canonical source paths corrects eight additional
sound-vtable bytes; all other bytes agree. This path-sensitive emission
was checked explicitly rather than assuming an isolated hash transfers.


#### Stage initializer frame allocation

Stage's constructor now copies its two camera snapshots with `memcpy`.
For this aggregate, the ordinary constructor's bytes and relocations are
identical to the previous implicit assignments. The source expression
changes the legacy compiler's failed-inlining stack accounting: the
standalone global initializer reserves `0x44` bytes, matching the original,
instead of `0x50`. No unused locals or stack padding were introduced.

The checkpoint hash above includes this change. It changes exactly one
executable byte from the previous `c4fe8d8e191a523ebc85dfef8bf72ec0d576e65d`
build; the total file-offset difference count remains 2,054 while the
initializer block is still shifted. Ordinary comparison remains 702/702.
Ascii and Player initializer frame reservations remained unresolved at
that checkpoint.
