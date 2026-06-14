# TBD Reforger Platform

Data-driven Arma Reforger event platform: missions are JSON from the website backend,
one greenfield Enfusion mod runs them all, and the web stack handles auth, events, and ORBAT.

**Repo:** [github.com/darkforce09/tbd-reforger-platform](https://github.com/darkforce09/tbd-reforger-platform)

---

## Status (2026-06-13)

| Area | Status |
|---|---|
| Phase 0 spikes | REST 0.1 ✓ · Registry 0.4 ✓ · Schema v1.0 ✓ |
| `tbd-framework/` | Mission loader, registry POC, game mode prefab, dev scenario |
| Dedicated server POC | ✓ Mission from API + 5 registry spawns on Linux host |
| Web backend (Phase 1 API) | ✓ Missions, link codes, roster, ORBAT slot assignment |
| **Phase 1 in progress** | Spawn/factions, ORBAT enforcement, capture objective, admin UI |
| Milestone #1 target | **Sat 2026-08-22** — see [`MILESTONES.md`](MILESTONES.md) |

---

## Quick start

### Claude Code (Enfusion work)

1. Read [`CLAUDE-CODE-START.md`](CLAUDE-CODE-START.md)
2. Enable **enfusion-mcp** before editing any `.c` file
3. Open **only** `tbd-framework/addon.gproj` in Workbench — never `Tbd_framework/`

### Dedicated server (local POC)

```bash
# Prereq: Steam app 1890870 (Arma Reforger Server), API on :8080, Postgres for website
bash scripts/setup-server-profile.sh          # default: .local-test-profile/
bash scripts/run-dev-server.sh                # -server + -addons (local mod)
```

Watch logs for `[TBD] Mission loaded` and `[TBD] Registry POC spawned` (×5).

### Website (local dev)

```bash
podman start tbdevent-postgres                # or docker compose up -d
cd Tbdevent_Website && make dev-api           # :8080
cd Tbdevent_Website/web && npm run dev        # :5173
bash scripts/test-phase1-api.sh
```

---

## Repository map

| Path | Purpose |
|---|---|
| [`tbd-framework/`](tbd-framework/) | **Production Enfusion mod** (TBD-owned) |
| [`tbd-schema/`](tbd-schema/) | Mission JSON schema, registry, golden missions, VOIP bridge contract |
| [`Tbdevent_Website/`](Tbdevent_Website/) | Go API + React UI |
| [`Tbd_framework/`](Tbd_framework/) | CRF reference only — **do not open in Workbench** |
| [`scripts/`](scripts/) | Workbench setup, server profile, dev server, API tests |
| [`docs/`](docs/) | Ops copy (Discord milestone post, etc.) |

**Handoff docs:** [`CLAUDE-CONTINUATION.md`](CLAUDE-CONTINUATION.md) · [`MILESTONES.md`](MILESTONES.md) · [`tbd-reforger-platform-build-plan.md`](tbd-reforger-platform-build-plan.md)

---

## Key IDs

| Item | Value |
|---|---|
| Mod GUID | `B2C3D4E5F6A78901` |
| Dev scenario | `{69A85365FC09E2CA}Missions/TBD_Dev_POC.conf` |
| Dev world | `{F652B97A6F497348}worlds/TBD_Dev_POC.ent` (Eden subscene) |
| Golden mission | `msn_8f3a2c` (Bridgehead at Levie) |
| Dev server port | `2001` (when using `-server` mode) |

---

## What not to do

- Do not open or ship `Tbd_framework/` (60+ Coalition deps)
- Do not guess Enfusion APIs — use enfusion-mcp
- Do not use `-config` and `-addons` together for local dev mods
- Payments / Stripe are out of scope; VOIP is partner-owned (external app)
