# th07's recipes are the shared ones from decomp-tools (tools/, a git
# submodule); the game's settings are in project.toml.
import 'tools/common.just'

# Fail unless the freshly built executable matches every original byte.
check-exe: exe
    python3 scripts/check_exe.py
