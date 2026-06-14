# Claude Code — start here

> **Read this file first** in Claude Code. Supersedes outdated API assumptions from 2023-era training data.
>
> **Workspace:** `/home/Samuel/Projects/Arma reforger`  
> **Last updated:** 2026-06-14  
> **Phase:** 1 (Milestone #1 target: Sat 2026-08-22)

---

## Critical rules

1. **Never guess Enfusion APIs.** Training data is ~3 years behind. Use **enfusion-mcp** (`api_search`, `wiki_read`) before every `.c` change.
2. **Production mod = `tbd-framework/` only.** Vanilla dep `58D0FB3206B6F859`. No Coalition GUIDs.
3. **`Tbd_framework/` is reference-only** — read patterns in Cursor; **never open in Workbench**, never add as dependency.
4. **Work on `main` only** — no feature branches.

---

## What is already done (do not redo)

- Schema **1.1** (`slots[]` required) · registry POC 0.4 · REST spike 0.1
- Mission loader: `GET /api/missions/{id}/compiled` + `$profile:missions/{id}.json` fallback
- **Per-slot spawn:** `TBD_SpawnManager` + modded `SCR_MenuSpawnLogic` — verified on staging dedicated server 2026-06-14
- Web Phase 1 **backend**: link codes, roster, mission publish, ORBAT slot assignment (migration 00004), **`GET /api/missions` list**
- Dev scenario: `Missions/TBD_Dev_POC.conf` on Eden subscene with `TBD_GameMode.et` prefab
- **Staging LAN join WORKS** — mod published to Workshop, `-config` mode, client Direct-Joined + spawned (see [`docs/STAGING-SERVER.md`](docs/STAGING-SERVER.md))
- **Mission browser** — backend + game logic + admin-gated client↔server RPC + keybind trigger all built & compile; only the 2 input actions remain (CLAUDE-CONTINUATION.md §16)
- Repo on GitHub: `darkforce09/tbd-reforger-platform`

---

## Phase 1 — what to build next

1. ~~**Staging LAN pass**~~ ✓ DONE 2026-06-14 — `tbd-framework` published to Workshop, staging runs `-config` mode, client Direct-Joined and spawned at a slot (see [`docs/STAGING-SERVER.md`](docs/STAGING-SERVER.md) → "Client join")
2. **Finish the mission browser** — define the 2 input actions (`TBD_MissionCycle`/`TBD_MissionLoad`) so the admin keybind fires the RPC. Everything else is done + compiles. Full handoff in [`CLAUDE-CONTINUATION.md`](CLAUDE-CONTINUATION.md) **§16**; read **§17 gotchas first**.
3. **Capture / win condition** — at least one objective
4. **Full ORBAT enforcement** — roster identity → assigned slot (round-robin works without linking)
5. **Stage machine** — `LOADING → LOBBY → BRIEFING → SAFE_START → LIVE → END → DEBRIEF`
6. **Web UI** — mission upload, slot assignment, identity linking (APIs exist)

See [`MILESTONES.md`](MILESTONES.md) for Milestone #1 success criteria.

---

## Step 0 — Claude Code + MCP

```bash
cd "/home/Samuel/Projects/Arma reforger"
claude mcp add --scope user enfusion-mcp -- npx -y enfusion-mcp
```

Cursor: `.cursor/mcp.json` with `ENFUSION_GAME_PATH`, `ENFUSION_WORKBENCH_PATH`, `ENFUSION_PROJECT_PATH`.

**Workbench verify (no wb_log in MCP):**

```bash
bash scripts/tbd-spawn-verify.sh
# or: mcp-call wb_play → mcp-wb-logs.sh → wb_stop
```

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

## Step 2 — Dedicated server (local or staging)

```bash
# Local:
bash scripts/setup-server-profile.sh
bash scripts/run-dev-server.sh

# Staging:
cp scripts/deploy.env.example scripts/deploy.env
bash scripts/deploy-staging.sh
```

**Important:** **Local** dev uses `-server` + `-addons` (do **not** pass `-config` with `-addons`). **Staging** now uses **`-config` mode** with the **Workshop** mod (`TBD_SERVER_MODE=config` in `deploy.env`) — that's the only Direct-Joinable mode; `-server`+`-addons` registers no backend room. `deploy-staging.sh` supports both (`TBD_SERVER_MODE=addons|config`).

**Success log lines:**

```
[TBD] Mission loaded from backend: Bridgehead at Levie
[TBD] Registry loaded
[TBD] SpawnManager: built slot spawn ... (×18)
[TBD] Stage → LOBBY
[TBD] SpawnManager: assigned slot ...
[TBD] SpawnManager: spawn requested
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
bash scripts/manual-test.sh
```

---

## Known script notes

- `RestCallback.SetOnTimeout` — **removed** from `TBD_MissionLoader.c` (not on current API)
- Prefer `JsonLoadContext` over obsolete `SCR_JsonLoadContext` when touching loader
- `TBD_GameMode.et` prefab includes `RplComponent` — do not duplicate in world layer

## Enfusion / Workbench gotchas (READ — cost hours on 2026-06-14; full list in CLAUDE-CONTINUATION.md §17)

- **`out` is a reserved keyword**; **no ternary `?:`** in Enforce.
- **The local dedicated-server compile check is UNRELIABLE for *new* .c files** (cached resourceDatabase.rdb skips them). **Verify compiles in Workbench** via the MCP: `wb_connect` → `wb_reload {scripts}` → grep `compatdata/1874910/.../ArmaReforgerWorkbench/logs/logs_*/console.log` for `Can't compile`/`(E):`.
- **Publishing packs `data.pak`+`meta` into the addon dir → Workbench marks it read-only** (padlock). Delete `tbd-framework/{data.pak,meta,ServerData.json,*_manifest.json}` (gitignored) after each publish, then restart the Launcher.
- **Chat is dead on the dedicated TBD scenario** (no `ScriptedChatEntity`). Admin input on dedicated = client→server RPC (`modded SCR_PlayerController` + `RplRpc`), not chat.
- enfusion-mcp **can't launch** Workbench here (Linux/Proton) — launch via Steam; then `project_read`/`project_write`/`wb_reload` work over the bridge.

---

## Repo map

| Path | Purpose |
|---|---|
| [`tbd-framework/`](tbd-framework/) | Enfusion mod — all new script work |
| [`tbd-schema/`](tbd-schema/) | Mission JSON 1.1, registry, bridge contract |
| [`Tbdevent_Website/`](Tbdevent_Website/) | Go API + React UI |
| [`docs/STAGING-SERVER.md`](docs/STAGING-SERVER.md) | Staging deploy guide |
| [`scripts/`](scripts/) | setup, deploy-staging, MCP helpers, tests |
| [`CLAUDE-CONTINUATION.md`](CLAUDE-CONTINUATION.md) | Full context, API table, team split |
| [`MILESTONES.md`](MILESTONES.md) | Milestone #1/#2 dates + gates |

---

## Suggested first prompt (new Claude chat)

### LAN join debug (current blocker)

```
Read CLAUDE-CODE-START.md, CLAUDE-CONTINUATION.md section 10, and docs/STAGING-SERVER.md.

Task: Find why Arma Reforger Direct Join to 192.168.0.140:2001 still fails (or confirm it works after the a2sPort fix).

Context:
- Staging server on sam@192.168.0.140 (WiFi); dev PC on LAN — same 192.168.0.0/24, ping OK.
- Phase A: -server + -addons B2C3D4E5F6A78901, NOT -config.
- Server logs show mission loaded, 18× built slot spawn, listening 0.0.0.0:2001.
- Client (Proton): launch options load tbd-framework mod (verify in compatdata/1874880 console.log).
- Fix already applied: -a2sPort 2001 (was 17777 — caused "No server found" because Direct Join probes :2001).
- scripts/deploy.env has SSH credentials (gitignored). Use scripts/debug-direct-join.sh for runtime checks.
- Never touch /home/sam/prairielearn/. Work on main only.

Investigate with runtime evidence:
1. bash scripts/debug-direct-join.sh before and after a join attempt
2. SSH: systemctl --user status tbd-reforger.service; tail server console.log during join
3. Client Proton console.log: SEARCHING_SERVER / SERVER_NOT_FOUND / version mismatch
4. Steam buildid: appmanifest 1874880 (client) vs 1874900 (server) — must match
5. If discovery works but connect fails: firewall, mod mismatch, BattlEye

Do not redo spawn manager or mission loader work. Commit only when I ask.
```

### General Phase 1 (after LAN join works)

```
Read CLAUDE-CODE-START.md and CLAUDE-CONTINUATION.md section 7.

Phase 1 toward Milestone #1 (2026-08-22). Slot spawn at JSON coords is done
(Workbench verified). Do NOT redo loader/spawn manager work.

Next task: [capture objective | ORBAT enforcement — pick one]

Rules:
- tbd-framework/ only, enfusion-mcp before any .c edit
- Never open Tbd_framework/ in Workbench
- main branch only
```

---

## Human ops

- Discord post draft: [`docs/discord-milestone-1-post.md`](docs/discord-milestone-1-post.md)
- Staging server: [`docs/STAGING-SERVER.md`](docs/STAGING-SERVER.md)
- Milestone #1 announced on website (Sat 22 Aug 2026)
