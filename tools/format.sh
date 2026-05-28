#!/bin/bash

# Copyright (c) 2025 HIGH CODE LLC
#
# TentacleOS is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# TentacleOS is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with TentacleOS. If not, see <https://www.gnu.org/licenses/>.


REPO_ROOT="$(git rev-parse --show-toplevel)"

TARGETS=(
  "$REPO_ROOT/firmware_p4/components"
  "$REPO_ROOT/firmware_p4/main"
  "$REPO_ROOT/firmware_c5/components"
  "$REPO_ROOT/firmware_c5/main"
)

EXCLUDE_DIRS=(
  "managed_components"
  "build"
)

EXCLUDE_ARGS=()
for dir in "${EXCLUDE_DIRS[@]}"; do
  EXCLUDE_ARGS+=(-not -path "*/$dir/*")
done

EXISTING_TARGETS=()
for target in "${TARGETS[@]}"; do
  if [ -d "$target" ]; then
    EXISTING_TARGETS+=("$target")
  fi
done

if [ ${#EXISTING_TARGETS[@]} -eq 0 ]; then
  echo "No target directories found."
  exit 0
fi

FILES=$(find "${EXISTING_TARGETS[@]}" \( -name "*.c" -o -name "*.h" \) "${EXCLUDE_ARGS[@]}")

if [ -z "$FILES" ]; then
  echo "No files to format."
  exit 0
fi

COUNT=$(echo "$FILES" | wc -l)

if [ "$1" = "--check" ]; then
  echo "Checking $COUNT files..."
  echo "$FILES" | xargs clang-format --dry-run --Werror 2>&1
  if [ $? -ne 0 ]; then
    echo "Formatting errors found. Run ./tools/format.sh to fix."
    exit 1
  fi
  echo "All files formatted correctly."
else
  echo "Formatting $COUNT files..."
  echo "$FILES" | xargs clang-format -i
  echo "Done."
fi
