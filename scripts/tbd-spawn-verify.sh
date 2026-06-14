#!/usr/bin/env bash
# Run Workbench Play and grep logs for slot spawn success.
# Usage: bash scripts/tbd-spawn-verify.sh [slot_id_substring]
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PATTERN="${1:-SpawnManager: built spawn point for|SpawnManager: assigned slot|TBD_SpawnLogic|built slot spawn}"
LOG_DIR="$HOME/Documents/Games/ArmaReforgerWorkbench/logs"
PROTON_LOG_DIR="$HOME/.local/share/Steam/steamapps/compatdata/1874910/pfx/drive_c/users/steamuser/Documents/My Games/ArmaReforgerWorkbench/logs"

bash "$ROOT/scripts/mcp-call.sh" wb_play '{}' || true
sleep 25
bash "$ROOT/scripts/mcp-call.sh" wb_stop '{}' || true

latest_log_dir() {
  local d picked=""
  for d in "$PROTON_LOG_DIR" "$LOG_DIR"; do
    [ -d "$d" ] || continue
    picked="$(ls -td "$d"/logs_* 2>/dev/null | head -1)"
    [ -n "$picked" ] && echo "$picked" && return
  done
}

LATEST="$(latest_log_dir)"
if [ -z "$LATEST" ]; then
  echo "No Workbench log directory found."
  exit 1
fi

echo "Log: $LATEST/console.log"
if grep -E "$PATTERN" "$LATEST/console.log" 2>/dev/null; then
  echo "PASS: spawn-related log lines found."
  exit 0
fi

echo "FAIL: expected log lines not found."
exit 1
