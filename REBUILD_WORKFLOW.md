# Rebuild Workflow for dev-slunsecore

This document explains how to maintain and build SlunseCore companion firmware with settings screen changes on top of the latest upstream MeshCore branch.

## Overview

The workflow ensures that:
1. Your `dev-slunsecore` branch always builds on top of the latest upstream `main`/`dev`
2. Upstream changes are synced to `origin/main` (or merged from `dev` per [WORKFLOW.md](WORKFLOW.md))
3. Your SlunseCore customizations are merged into `dev-slunsecore`
4. GitHub Actions automatically builds settings-screen firmware targets on push

## Current Workflow (Recommended)

### Simple 2-Step Process

```bash
# Step 1: Update dev-slunsecore with latest upstream
./rebuild-with-settings.sh

# Step 2: Push to trigger GitHub Actions build
git push origin dev-slunsecore
```

That's it! GitHub Actions will:
- Detect version from source code
- Build all 6 firmware targets
- Create a pre-release with all files

## Scripts

### 1. `rebuild-with-settings.sh` (Primary Script)

**Usage:**
```bash
./rebuild-with-settings.sh

# With auto-push (triggers GitHub Actions)
AUTO_PUSH=true ./rebuild-with-settings.sh
```

**What it does:**
1. Fetches latest from origin and upstream
2. Syncs upstream/main to origin/main (if needed)
3. Updates local main branch
4. Merges main into dev-slunsecore
5. Preserves your customizations on top of latest upstream

**After running:**
- Push manually: `git push origin dev-slunsecore`
- GitHub Actions automatically builds all firmwares

### 2. `auto-rebuild-settings.sh` (For Automation)

Lightweight wrapper around `rebuild-with-settings.sh` for cron/CI.

**Usage:**
```bash
# Basic usage
./auto-rebuild-settings.sh

# With auto-push
./auto-rebuild-settings.sh --push
```

## Branch Strategy

```
upstream/main  ──────────────┐
                              │
origin/main    ───────────────┼───┐
                              │   │
upstream/dev   ───────────────┼───┼───> dev-slunsecore (SlunseCore customizations)
                              │   │           ↓ (push triggers)
                              │   │       GitHub Actions (settings-screen builds)
                              │   │
                              │   └───> slunsecore (production releases)
```

### Branch Purposes

- **`main`**: Synced with upstream (reference)
- **`dev-slunsecore`**: Integration branch (upstream + SlunseCore customizations)
  - Settings-screen CI builds run on push
  - Test here before merging to `slunsecore`
- **`slunsecore`**: Production release branch (see [WORKFLOW.md](WORKFLOW.md))

## Workflow Options

### Option 1: Manual Update (Recommended)

```bash
# 1. Update dev-slunsecore
./rebuild-with-settings.sh

# 2. Review changes (if any conflicts, resolve them)
git log --oneline -5

# 3. Push to trigger build
git push origin dev-slunsecore
```

### Option 2: Automated Update (Cron/CI)

```bash
# Cron example (daily at 2 AM)
0 2 * * * cd /path/to/MeshCore && ./auto-rebuild-settings.sh --push >> /var/log/meshcore-update.log 2>&1
```

### Option 3: Integration with sync-all-repos.sh

See `sync-integration-example.sh` for a snippet that runs `rebuild-with-settings.sh` after syncing MeshCore.

## Merge vs Rebase

This workflow uses **merge** (not rebase) to preserve history.

**If conflicts occur:**
1. Resolve conflicts manually
2. `git add <resolved-files>`
3. `git commit -m "Resolve conflicts with upstream vX.X.X"`
4. Push: `git push origin dev-slunsecore`

## What Gets Built

When you push to `dev-slunsecore`, GitHub Actions automatically:

1. **Detects version** from source code (e.g., `#define FIRMWARE_VERSION "v1.16.0"`)
2. **Builds 6 firmware targets**:
   - RAK_4631_companion_radio_ble
   - RAK_4631_companion_radio_usb
   - Heltec_v3_companion_radio_ble
   - Heltec_v3_companion_radio_usb
   - Heltec_v4_companion_radio_ble
   - Heltec_v4_companion_radio_usb
3. **Creates pre-release** tagged `dev-slunsecore-<commit>`

## Troubleshooting

### Merge Conflicts

If merge fails with conflicts:
1. Resolve conflicts in the affected files
2. `git add <resolved-files>`
3. `git commit -m "Resolve conflicts with upstream vX.X.X"`
4. Push: `git push origin dev-slunsecore`

### Build Failures

Check GitHub Actions logs: https://github.com/aXistem-dev/MeshCore/actions

## Quick Reference

| Action | Command |
|--------|---------|
| Update dev-slunsecore | `./rebuild-with-settings.sh` |
| Push to build | `git push origin dev-slunsecore` |
| Check builds | https://github.com/aXistem-dev/MeshCore/actions |
| Production release | See [WORKFLOW.md](WORKFLOW.md) and [RELEASE.md](RELEASE.md) |
