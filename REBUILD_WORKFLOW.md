# Rebuild Workflow for dev-cnfi

This document explains how to maintain and build your custom firmware with the settings screen changes on top of the latest upstream main branch.

## Overview

The workflow ensures that:
1. Your `dev-cnfi` branch always builds on top of the latest `main` from upstream
2. Upstream changes are automatically synced to `origin/main`
3. Your customizations are merged into `dev-cnfi`
4. GitHub Actions automatically builds all firmware targets on push

## Current Workflow (Recommended)

### Simple 2-Step Process

```bash
# Step 1: Update dev-cnfi with latest upstream
./update-dev-cnfi.sh

# Step 2: Push to trigger GitHub Actions build
git push origin dev-cnfi
```

That's it! GitHub Actions will:
- Detect version from source code
- Build all 6 firmware targets
- Create release with all files

## Scripts

### 1. `update-dev-cnfi.sh` (Primary Script) ⭐

**This is the main script you should use.**

**Usage:**
```bash
./update-dev-cnfi.sh
```

**What it does:**
1. Fetches latest from origin and upstream
2. Syncs upstream/main to origin/main (if needed)
3. Updates local main branch
4. Merges main into dev-cnfi
5. Preserves your customizations on top of latest upstream

**After running:**
- Push manually: `git push origin dev-cnfi`
- GitHub Actions automatically builds all firmwares

### 2. `rebuild-with-settings.sh` (Alternative)

Full-featured script that does sync + merge + optional push.

**Usage:**
```bash
# Without auto-push
./rebuild-with-settings.sh

# With auto-push (triggers GitHub Actions)
AUTO_PUSH=true ./rebuild-with-settings.sh
```

**What it does:**
1. Checks if upstream has updates
2. Updates origin/main from upstream
3. Merges main into dev-cnfi
4. Optionally pushes to trigger GitHub Actions

### 3. `auto-rebuild-settings.sh` (For Automation)

Lightweight wrapper around `update-dev-cnfi.sh` for cron/CI.

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
                              │   └───> dev-cnfi (your customizations)
                              │           ↓ (push triggers)
                              │       GitHub Actions (auto-builds)
                              │
                              └───> dev-settingsscreen (historical, don't modify)
```

### Branch Purposes

- **`main`**: Always synced with upstream (no customizations)
- **`dev-settingsscreen`**: Historical reference (v1.10.0 based, preserved)
- **`dev-cnfi`**: Working branch (latest main + your customizations)
  - This is what gets built
  - This is what you work on
  - This is what gets deployed

## Workflow Options

### Option 1: Manual Update (Recommended)

```bash
# 1. Update dev-cnfi
./update-dev-cnfi.sh

# 2. Review changes (if any conflicts, resolve them)
git log --oneline -5

# 3. Push to trigger build
git push origin dev-cnfi
```

### Option 2: Automated Update (Cron/CI)

Set up a cron job to automatically update and rebuild:

```bash
# Cron example (daily at 2 AM)
0 2 * * * cd /path/to/MeshCore && ./auto-rebuild-settings.sh --push >> /var/log/meshcore-update.log 2>&1
```

### Option 3: Integration with sync-all-repos.sh

Modify your `sync-all-repos.sh` to automatically update dev-cnfi after syncing:

```bash
# After syncing MeshCore, update dev-cnfi
if [ "$repo_dir" = "MeshCore" ] && [ "$was_updated" = true ]; then
    echo -e "\n${BLUE}🔄 Updating dev-cnfi with settings screen...${NC}"
    cd "$repo_path"
    ./update-dev-cnfi.sh
    git push origin dev-cnfi  # Triggers GitHub Actions
    cd "$BASE_DIR"
fi
```

## Merge vs Rebase

This workflow uses **merge** (not rebase) to preserve history:
- ✅ Preserves commit history
- ✅ Easier to track what came from where
- ✅ Less risk of losing work
- ✅ Can see the "merge point" clearly

**If conflicts occur:**
1. Resolve conflicts manually
2. `git add <resolved-files>`
3. `git commit -m "Resolve conflicts with upstream vX.X.X"`
4. Push: `git push origin dev-cnfi`

## What Gets Built

When you push to `dev-cnfi`, GitHub Actions automatically:

1. **Detects version** from source code (e.g., `#define FIRMWARE_VERSION "v1.12.0"`)
2. **Builds 6 firmware targets**:
   - RAK_4631_companion_radio_ble (creates .zip, .uf2)
   - RAK_4631_companion_radio_usb (creates .zip, .uf2)
   - Heltec_v3_companion_radio_ble (creates .bin, -merged.bin)
   - Heltec_v3_companion_radio_usb (creates .bin, -merged.bin)
   - Heltec_v4_companion_radio_ble (creates .bin, -merged.bin)
   - Heltec_v4_companion_radio_usb (creates .bin, -merged.bin)
3. **Creates release** with all 12 files
4. **Names files** as: `device-version-dev-settingsscreen-commithash.ext`

## Version Detection

The workflow automatically:
- Extracts version from source code (`FIRMWARE_VERSION` in `MyMesh.h` files)
- Compares with git tags
- Uses the **higher** version
- **Future-proof**: When main moves to 1.12.0, it automatically uses 1.12.0

## Troubleshooting

### Merge Conflicts

If merge fails with conflicts:
1. Resolve conflicts in the affected files
2. `git add <resolved-files>`
3. `git commit -m "Resolve conflicts with upstream vX.X.X"`
4. Push: `git push origin dev-cnfi`

### Build Failures

If GitHub Actions build fails:
- Check GitHub Actions logs: https://github.com/axistem-dev/MeshCore/actions
- Verify version detection (should show in logs)
- Check that all files are being built (debug output shows this)

### Upstream Sync Issues

If upstream sync fails:
1. Check network connectivity
2. Verify upstream remote is configured: `git remote -v`
3. Check upstream repository access

## Best Practices

1. **Always review changes** after merge to ensure nothing broke
2. **Test locally** before pushing (optional, GitHub Actions will build)
3. **Keep commits clean** - your customizations are preserved
4. **Monitor GitHub Actions** - builds happen automatically on push
5. **Document changes** - keep notes on what your branch adds/modifies

## Example Workflow

```bash
# 1. Sync upstream (if not done automatically)
git fetch upstream
git checkout main
git merge upstream/main
git push origin main

# 2. Update dev-cnfi with customizations
./update-dev-cnfi.sh

# 3. Push to trigger build
git push origin dev-cnfi

# 4. Check build status
# Visit: https://github.com/axistem-dev/MeshCore/actions
```

## Quick Reference

| Action | Command |
|--------|---------|
| Update dev-cnfi | `./update-dev-cnfi.sh` |
| Push to build | `git push origin dev-cnfi` |
| Check builds | https://github.com/axistem-dev/MeshCore/actions |
| Sync upstream | `git fetch upstream && git checkout main && git merge upstream/main && git push origin main` |
