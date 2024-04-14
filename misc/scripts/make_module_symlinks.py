#!/bin/python3
import sys
from pathlib import Path

modules_folder = Path("modules")
engine_modules_folder = Path("engine/modules")
for module_folder in modules_folder.iterdir():
    if module_folder.is_dir():
        symlink_target = engine_modules_folder / module_folder.parts[-1]
        if symlink_target.exists():
            if symlink_target.is_symlink():
                symlink_target.unlink()
            else:
                print(f"There was an issue with the symlink to {symlink_target}, it exists but is not a symlink")
                sys.exit(-1)
            symlink_target.symlink_to(module_folder)