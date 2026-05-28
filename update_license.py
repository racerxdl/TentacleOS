#!/usr/bin/env python3
import os

ROOT = os.path.dirname(os.path.abspath(__file__))

SKIP = {"build", "managed_components", "lvgl-env"}

# --- Replacement pairs (old Apache block → new GPL v3 block) -----------------

SLASH_OLD = (
    "// Licensed under the Apache License, Version 2.0 (the \"License\");\n"
    "// you may not use this file except in compliance with the License.\n"
    "// You may obtain a copy of the License at\n"
    "//\n"
    "//     http://www.apache.org/licenses/LICENSE-2.0\n"
    "//\n"
    "// Unless required by applicable law or agreed to in writing, software\n"
    "// distributed under the License is distributed on an \"AS IS\" BASIS,\n"
    "// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n"
    "// See the License for the specific language governing permissions and\n"
    "// limitations under the License."
)

SLASH_NEW = (
    "// TentacleOS is free software: you can redistribute it and/or modify\n"
    "// it under the terms of the GNU General Public License as published by\n"
    "// the Free Software Foundation, either version 3 of the License, or\n"
    "// (at your option) any later version.\n"
    "//\n"
    "// TentacleOS is distributed in the hope that it will be useful,\n"
    "// but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    "// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n"
    "// GNU General Public License for more details.\n"
    "//\n"
    "// You should have received a copy of the GNU General Public License\n"
    "// along with TentacleOS. If not, see <https://www.gnu.org/licenses/>."
)

HASH_OLD = (
    "# Licensed under the Apache License, Version 2.0 (the \"License\");\n"
    "# you may not use this file except in compliance with the License.\n"
    "# You may obtain a copy of the License at\n"
    "#\n"
    "# http://www.apache.org/licenses/LICENSE-2.0\n"
    "#\n"
    "# Unless required by applicable law or agreed to in writing, software\n"
    "# distributed under the License is distributed on an \"AS IS\" BASIS,\n"
    "# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n"
    "# See the License for the specific language governing permissions and\n"
    "# limitations under the License."
)

HASH_NEW = (
    "# TentacleOS is free software: you can redistribute it and/or modify\n"
    "# it under the terms of the GNU General Public License as published by\n"
    "# the Free Software Foundation, either version 3 of the License, or\n"
    "# (at your option) any later version.\n"
    "#\n"
    "# TentacleOS is distributed in the hope that it will be useful,\n"
    "# but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    "# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n"
    "# GNU General Public License for more details.\n"
    "#\n"
    "# You should have received a copy of the GNU General Public License\n"
    "# along with TentacleOS. If not, see <https://www.gnu.org/licenses/>."
)

REPLACEMENTS = [(SLASH_OLD, SLASH_NEW), (HASH_OLD, HASH_NEW)]

# --- Header to insert in tools/ scripts (no existing copyright) --------------

HASH_HEADER = (
    "# Copyright (c) 2025 HIGH CODE LLC\n"
    "#\n"
    "# TentacleOS is free software: you can redistribute it and/or modify\n"
    "# it under the terms of the GNU General Public License as published by\n"
    "# the Free Software Foundation, either version 3 of the License, or\n"
    "# (at your option) any later version.\n"
    "#\n"
    "# TentacleOS is distributed in the hope that it will be useful,\n"
    "# but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    "# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n"
    "# GNU General Public License for more details.\n"
    "#\n"
    "# You should have received a copy of the GNU General Public License\n"
    "# along with TentacleOS. If not, see <https://www.gnu.org/licenses/>."
)

TOOLS_SKIP = {"LVGLImage.py"}  # third-party
TOOLS_EXTS = {".sh", ".py", ".ps1"}

# -----------------------------------------------------------------------------

def replace_in_file(fpath):
    with open(fpath, "r", encoding="utf-8", errors="ignore") as f:
        content = f.read()
    if "Copyright (c) 2025 HIGH CODE LLC" not in content:
        return False
    new_content = content
    for old, new in REPLACEMENTS:
        new_content = new_content.replace(old, new)
    if new_content == content:
        return False
    with open(fpath, "w", encoding="utf-8") as f:
        f.write(new_content)
    return True


def insert_header_in_file(fpath):
    with open(fpath, "r", encoding="utf-8", errors="ignore") as f:
        content = f.read()
    if "Copyright" in content:
        return False
    lines = content.splitlines(keepends=True)
    if lines and lines[0].startswith("#!"):
        new_content = lines[0] + "\n" + HASH_HEADER + "\n\n" + "".join(lines[1:])
    else:
        new_content = HASH_HEADER + "\n\n" + content
    with open(fpath, "w", encoding="utf-8") as f:
        f.write(new_content)
    return True


updated = 0
skipped = 0

# Pass 1 — replace Apache → GPL v3 in firmware sources
for subdir in ["firmware_c5/main", "firmware_c5/components", "firmware_p4/main", "firmware_p4/components"]:
    for dirpath, dirnames, filenames in os.walk(os.path.join(ROOT, subdir)):
        dirnames[:] = [d for d in dirnames if d not in SKIP]
        for fname in filenames:
            ext = os.path.splitext(fname)[1]
            if ext not in {".c", ".h"} and fname != "CMakeLists.txt":
                continue
            fpath = os.path.join(dirpath, fname)
            if replace_in_file(fpath):
                print(f"  replaced: {os.path.relpath(fpath, ROOT)}")
                updated += 1
            else:
                skipped += 1

# Pass 2 — insert GPL v3 header in tools/ scripts
for dirpath, dirnames, filenames in os.walk(os.path.join(ROOT, "tools")):
    dirnames[:] = [d for d in dirnames if d not in SKIP]
    for fname in filenames:
        if fname in TOOLS_SKIP:
            continue
        if os.path.splitext(fname)[1] not in TOOLS_EXTS:
            continue
        fpath = os.path.join(dirpath, fname)
        if insert_header_in_file(fpath):
            print(f"  inserted: {os.path.relpath(fpath, ROOT)}")
            updated += 1
        else:
            skipped += 1

print(f"\n{updated} files updated, {skipped} skipped.")
