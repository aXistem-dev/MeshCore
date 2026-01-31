# Rebuild Workflow for dev-slunsecore

This document explains how to maintain and build SlunseCore firmware (settings screen, boot screen, timezone, screensaver) on top of the latest upstream main branch.

## Overview

The workflow ensures that:
1. Your `dev-slunsecore` branch builds on top of the latest `main` from upstream
2. Upstream changes are synced to `origin/main`
3. Your SlunseCore customizations are merged into `dev-slunsecore`
4. GitHub Actions builds settings screen firmwares on push to `dev-slunsecore`

## Simple 2-Step Process

```bash
# Step 1: Update dev-slunsecore with latest upstream
./update-dev-slunsecore.sh   # or ./rebuild-with-settings.sh

# Step 2: Push to trigger GitHub Actions build
git push origin dev-slunsecore
```

GitHub Actions will detect version from source, build all settings screen targets, and create a release.

## Scripts

### 1. `update-dev-slunsecore.sh` (Primary)

**Usage:** `./update-dev-slunsecore.sh`

Fetches upstream, syncs main, merges main into dev-slunsecore. After running, push manually to trigger build.

### 2. `rebuild-with-settings.sh` (Manual)

**Usage:** `./rebuild-with-settings.sh` or `AUTO_PUSH=true ./rebuild-with-settings.sh`

Syncs upstream/main to origin/main, merges main into dev-slunsecore, optionally pushes.

### 3. `auto-rebuild-settings.sh` (Automation)

**Usage:** `./auto-rebuild-settings.sh` or `./auto-rebuild-settings.sh --push`

Calls update-dev-slunsecore.sh if present, otherwise fetches and merges; optionally pushes.

## Branch Strategy

```
upstream/main  ──────────────┐
                              │
origin/main    ───────────────┼───┐
                              │   │
                              │   └───> dev-slunsecore (SlunseCore: settings + boot screen)
                              │           ↓ (push triggers)
                              │       GitHub Actions (auto-builds)
                              └───> (other branches)
```

- **`main`**: Synced with upstream (no customizations)
- **`dev-slunsecore`**: Working branch (latest main + SlunseCore features: settings menu, boot screen, timezone, screensaver)

## Workflow Options

### Option 1: Manual Update

```bash
./update-dev-slunsecore.sh   # or rebuild-with-settings.sh
git log --oneline -5
git push origin dev-slunsecore
```

### Option 2: Automated (Cron)

```bash
0 2 * * * cd /path/to/MeshCore && ./auto-rebuild-settings.sh --push >> /var/log/meshcore-update.log 2>&1
```

### Option 3: Integration with sync-all-repos.sh

See `sync-integration-example.sh` for a snippet that updates dev-slunsecore after syncing MeshCore.

## Merge Conflicts

If merge fails:
1. Resolve conflicts in the affected files
2. `git add <resolved-files>`
3. `git commit -m "Resolve conflicts with upstream"`
4. `git push origin dev-slunsecore`

## What Gets Built

When you push to `dev-slunsecore`, GitHub Actions (build-settingsscreen-releases.yml):

1. **Detects version** from source code
2. **Builds 6 firmware targets** (RAK_4631 BLE/USB, Heltec v3/v4 BLE/USB)
3. **Creates release** with files named `device-version-dev-slunsecore-commithash.ext`

## Version Detection

- Extracts version from source (`FIRMWARE_VERSION` in examples)
- Compares with git tags; uses the higher version

## Troubleshooting

### Merge Conflicts

Resolve, `git add`, `git commit`, then `git push origin dev-slunsecore`.

### Build Failures

Check GitHub Actions: https://github.com/axistem-dev/MeshCore/actions

### Upstream Sync Issues

If upstream sync fails: check network, verify `git remote -v` has upstream, check repository access.

## Best Practices

1. **Review changes** after merge before pushing
2. **Test locally** if possible; GitHub Actions will build on push
3. **Keep dev-slunsecore** as the single branch for SlunseCore features
4. **Document changes** – note what your branch adds/modifies

## Quick Reference

| Action | Command |
|--------|---------|
| Update dev-slunsecore | `./update-dev-slunsecore.sh` or `./rebuild-with-settings.sh` |
| Push to build | `git push origin dev-slunsecore` |
| Check builds | https://github.com/axistem-dev/MeshCore/actions |
| Sync upstream | `git fetch upstream && git checkout main && git merge upstream/main && git push origin main` |
