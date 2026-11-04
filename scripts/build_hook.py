"""th07's standalone translation-unit ordering beyond address sorting."""
from pathlib import Path
import sys


def exe_objects(objs: list[str], feats: list[str]) -> list[str]:
    """Preserve the original ordering of code and data ownership units.

    Address sorting puts data-only units last. The strings in filemodes
    must precede File's vtables in the reversed read-only object pool;
    its initialized pointer array retains File's original data position.
    """
    scripts = str(Path(__file__).resolve().parent)
    if scripts not in sys.path:
        sys.path.insert(0, scripts)
    from crt_archives import prepare_archives

    prepare_archives()
    modes = next(obj for obj in objs if Path(obj).name == "filemodes.obj")
    ordered = [obj for obj in objs if obj != modes]
    after = next(i for i, obj in enumerate(ordered) if Path(obj).name == "file.obj") + 1
    ordered.insert(after, modes)
    # Uninitialized data follows object order; the archive array's
    # constructor remains in the original unit before these data-only units.
    data_names = ("renderer_globals.obj", "replay_globals.obj",
                  "input_globals.obj", "file_globals.obj")
    data_units = [next(obj for obj in ordered if Path(obj).name == name)
                  for name in data_names]
    ordered = [obj for obj in ordered if obj not in data_units]
    after = next(i for i, obj in enumerate(ordered)
                 if Path(obj).name == "unsorted4b9e44.obj") + 1
    ordered[after:after] = data_units
    return ordered
