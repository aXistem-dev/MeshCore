# Complete Workflow: Sync → Apply Customizations → Rebuild

## Overview

This document describes the complete workflow for maintaining your settings screen customizations on top of the latest upstream code.

## The Complete Flow

```
Upstream (external repo)
    ↓ (sync)
Origin/main (your fork's main)
    ↓ (merge)
dev-cnfi (your customizations)
    ↓ (push triggers)
GitHub Actions (auto-builds all firmwares)
```

## Step-by-Step Workflow

### 1. Sync from Upstream to Origin

**When**: Whenever upstream releases a new version (e.g., v1.12.0)

```bash
# Fetch latest from upstream
git fetch upstream

# Update your origin/main from upstream/main
git checkout main
git merge upstream/main  # or: git pull upstream main
git push origin main
```

**What happens**:
- Your `origin/main` gets the latest upstream code
- New version (e.g., 1.12.0) is now in your main branch

### 2. Apply Settings Screen Customizations

**When**: After syncing upstream (step 1)

```bash
# Use the update script
./update-dev-cnfi.sh
```

**What the script does**:
1. Fetches latest from origin and upstream
2. Updates `main` from `origin/main`
3. Merges `main` into `dev-cnfi`
4. Your customizations are automatically preserved on top of the new version

**Important**: Your customizations are **always preserved**. They exist as commits in `dev-cnfi` and will remain even when upstream updates to v1.12.0, v1.13.0, etc. See `CUSTOMIZATIONS_PRESERVATION.md` for details.

**If conflicts occur**:
```bash
# Resolve conflicts manually
git add .
git commit -m "Resolve conflicts with upstream v1.12.0"
```

### 3. Rebuild (Automatic via GitHub Actions)

**When**: After pushing `dev-cnfi` (step 2)

```bash
# Push dev-cnfi to trigger build
git push origin dev-cnfi
```

**What happens automatically**:
1. GitHub Actions detects push to `dev-cnfi`
2. Extracts version from source code (e.g., 1.12.0)
3. Builds all 6 firmware targets:
   - RAK_4631_companion_radio_ble/usb
   - Heltec_v3_companion_radio_ble/usb
   - Heltec_v4_companion_radio_ble/usb
4. Creates release with all firmware files
5. Files named: `device-1.12.0-dev-settingsscreen-commithash.ext`

## Complete Workflow Script

Here's a single script that does everything:

```bash
#!/bin/bash
# complete-sync-and-rebuild.sh

set -e

echo "=== Complete Sync and Rebuild Workflow ==="
echo ""

# Step 1: Sync upstream to origin/main
echo "Step 1: Syncing upstream to origin/main..."
git fetch upstream
git checkout main
git merge upstream/main
git push origin main
echo "✓ Main updated"

# Step 2: Update dev-cnfi with latest main + customizations
echo ""
echo "Step 2: Updating dev-cnfi with customizations..."
./update-dev-cnfi.sh

# Step 3: Push to trigger GitHub Actions build
echo ""
echo "Step 3: Pushing dev-cnfi to trigger build..."
git push origin dev-cnfi
echo "✓ Build triggered"

echo ""
echo "=== Workflow Complete ==="
echo "Check GitHub Actions for build status:"
echo "https://github.com/axistem-dev/MeshCore/actions"
```

## Manual Workflow (Step-by-Step)

If you prefer to do it manually:

### Step 1: Sync Upstream
```bash
git fetch upstream
git checkout main
git merge upstream/main
git push origin main
```

### Step 2: Update dev-cnfi
```bash
./update-dev-cnfi.sh
# Or manually:
# git checkout dev-cnfi
# git merge main
# (resolve conflicts if any)
```

### Step 3: Push and Build
```bash
git push origin dev-cnfi
# GitHub Actions automatically builds
```

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

## Branch Strategy Summary

- **`main`**: Always synced with upstream (no customizations)
- **`dev-settingsscreen`**: Historical reference (v1.10.0 based, don't modify)
- **`dev-cnfi`**: Working branch (latest main + your customizations)
  - This is what gets built
  - This is what you work on
  - This is what gets deployed

## Quick Reference

| Action | Command |
|--------|---------|
| Sync upstream | `git fetch upstream && git checkout main && git merge upstream/main && git push origin main` |
| Update dev-cnfi | `./update-dev-cnfi.sh` |
| Push to build | `git push origin dev-cnfi` |
| Check builds | https://github.com/axistem-dev/MeshCore/actions |

## Troubleshooting

### Conflicts during merge
```bash
# Resolve conflicts, then:
git add .
git commit -m "Resolve conflicts with upstream vX.X.X"
git push origin dev-cnfi
```

### Build fails
- Check GitHub Actions logs
- Verify version detection (should show in logs)
- Check that all files are being built (debug output shows this)

### Wrong version in filenames
- Version is extracted from source code automatically
- If source has `v1.12.0`, filenames will use `1.12.0`
- Check `examples/companion_radio/MyMesh.h` for `FIRMWARE_VERSION` definition

