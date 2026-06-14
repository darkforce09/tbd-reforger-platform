# Claude Code — start here

> **Read this file first** in Claude Code. Supersedes outdated API assumptions from 2023-era training data.
>
> **Workspace:** `/home/Samuel/Projects/Arma reforger`  
> **Last updated:** 2026-06-13  
> **Phase:** 1 (Milestone #1 target: Sat 2026-08-22)

---

## Critical rules

1. **Never guess Enfusion APIs.** Training data is ~3 years behind. Use **enfusion-mcp** (`api_search`, `wiki_read`) before every `.c` change.
2. **Production mod = `tbd-framework/` only.** Vanilla dep `58D0FB3206B6F859`. No Coalition GUIDs.
3. **`Tbd_framework/` is reference-only** — read patterns in Cursor; **never open in Workbench**, never add as dependency.

---

## What is already done (do not redo)

- Schema v1.0 frozen · registry POC 0.4 · REST spike 0.1
- Mission loader: `GET /api/missions/{id}/compiled` + `$profile:missions/{id}.json` fallback
- Dedicated server POC: 5 registry spawns + mission from live API (Linux, `-server` + `-addons`)
- Web Phase 1 **backend**: link codes, roster, mission publish, ORBAT slot assignment (migration 00004)
- Dev scenario: `Missions/TBD_Dev_POC.conf` on Eden subscene with `TBD_GameMode.et` prefab
- Repo on GitHub: `darkforce09/tbd-reforger-platform`

---

## Phase 1 — what to build next

1. **Player spawn** — `SCR_SpawnLogic`, factions, spawn points on land (not `<0,1,0>`)
2. **ORBAT enforcement** — `GET /api/game/events/{id}/roster` on join
3. **Capture / win condition** — at least one objective
4. **Stage machine** — `LOADING → LOBBY → BRIEFING → SAFE_START → LIVE → END → DEBRIEF`
5. **Admin commands** — `#stage next`, `#end blufor`, etc.
6. **Web UI** — mission upload, slot assignment, identity linking (APIs exist)

See [`MILESTONES.md`](MILESTONES.md) for Milestone #1 success criteria.

---

## Step 0 — Claude Code + MCP

```bash
cd "/home/Samuel/Projects/Arma reforger"
claude mcp add --scope user enfusion-mcp -- npx -y enfusion-mcp
```

Cursor: `.cursor/mcp.json` (enfusion-mcp already configured).

---

## Step 1 — Workbench (when editing scripts)

```bash
bash scripts/setup-workbench-linux.sh   # base game symlink for Proton/Linux
```

| Open | Do not open |
|---|---|
| `~/ArmaReforger-Base/data/ArmaReforger.gproj` | — |
| `tbd-framework/addon.gproj` | `Tbd_framework/addon.gproj` |

Sync **Arma Reforger** and **Arma Reforger Tools** to the same Steam build (mismatch → vanilla `Tuple2` errors).

---

## Step 2 — Dedicated server POC (verify loader + registry)

```bash
# Install server once: Steam app 1890870
bash scripts/setup-server-profile.sh     # writes .local-test-profile/profile/*
bash scripts/run-dev-server.sh           # listens on :2001
```

**Important:** Local mods use `-server` + `-addons`. Do **not** pass `-config` with `-addons` (engine rejects it). Workshop-published mods go in `game.mods` inside config JSON instead.

**Profile layout** — `$profile:` maps to `<profileDir>/profile/`:

```
.local-test-profile/profile/
  TBD_BackendConfig.json
  TBD_Registry.json          # optional override
  missions/msn_8f3a2c.json   # cached after REST fetch
```

**Success log lines:**

```
[TBD] Mission loaded from backend: Bridgehead at Levie
[TBD] Registry loaded (5 aliases).
[TBD] Registry POC spawned kit:us_rifleman at ...
(... ×5)
NETWORK : Starting RPL server, listening on address 0.0.0.0:2001
```

---

## Step 3 — Web API (already running from prior work)

| Service | URL |
|---|---|
| API | http://127.0.0.1:8080 |
| UI dev | http://127.0.0.1:5173 |
| Postgres | `podman start tbdevent-postgres` (port **5433** in `.env`) |

```bash
bash scripts/test-phase1-api.sh
bash scripts/manual-test.sh              # full suite (needs DB)
```

Migration **00004** applied: missions, link codes, game identities, event slot assignments.

---

## Known script notes

- `RestCallback.SetOnTimeout` — **removed** from `TBD_MissionLoader.c` (not on current API)
- Prefer `JsonLoadContext` over obsolete `SCR_JsonLoadContext` when touching loader
- `TBD_GameMode.et` prefab includes `RplComponent` — do not duplicate in world layer

---

## Repo map

| Path | Purpose |
|---|---|
| [`tbd-framework/`](tbd-framework/) | Enfusion mod — all new script work |
| [`tbd-schema/`](tbd-schema/) | Mission JSON, registry, bridge contract |
| [`Tbdevent_Website/`](Tbdevent_Website/) | Go API + React UI |
| [`Tbd_framework/`](Tbd_framework/) | CRF reference — do not ship |
| [`scripts/`](scripts/) | setup-workbench, setup-server-profile, run-dev-server, tests |
| [`CLAUDE-CONTINUATION.md`](CLAUDE-CONTINUATION.md) | Full context, API table, team split |
| [`MILESTONES.md`](MILESTONES.md) | Milestone #1/#2 dates + gates |
| [`README.md`](README.md) | Project overview |

---

## Suggested first prompt (new Claude chat)

```
Read CLAUDE-CODE-START.md and CLAUDE-CONTINUATION.md section 7.

Phase 1 toward Milestone #1 (2026-08-22). Dedicated server POC is done
(mission from API + registry ×5). Do NOT redo loader/registry work.

Next task: [spawn/factions | ORBAT enforcement | capture objective — pick one]

Rules:
- tbd-framework/ only, enfusion-mcp before any .c edit
- Never open Tbd_framework/ in Workbench
```

---

## Human ops

- Discord post draft: [`docs/discord-milestone-1-post.md`](docs/discord-milestone-1-post.md)
- Milestone #1 announced on website (Sat 22 Aug 2026) — post Discord when ready
