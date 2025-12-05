# Rebuild Workflow for dev-settingsscreen

This document explains how to maintain and build your custom firmware with the settings screen changes on top of the latest upstream main branch.

## Overview

The workflow ensures that:
1. Your `dev-settingsscreen` branch always builds on top of the latest `main` from upstream
2. Upstream changes are automatically synced to `origin/main`
3. Your feature branch is rebased onto the updated main
4. Firmware is built with your custom changes

## Scripts

### 1. `rebuild-with-settings.sh` (Manual/Interactive)

Full-featured script for manual rebuilds with detailed output.

**Usage:**
```bash
cd MeshCore
./rebuild-with-settings.sh
```

**What it does:**
1. Checks if upstream/main has new commits
2. Updates origin/main from upstream
3. Rebases dev-settingsscreen onto main
4. Builds firmware for RAK_4631_companion_radio_ble
5. Optionally pushes the rebased branch

**Configuration:**
Edit the script to change:
- `BUILD_TARGET`: Change the firmware target to build
- `AUTO_PUSH`: Set to `true` to automatically push after rebase

### 2. `auto-rebuild-settings.sh` (Automated)

Lightweight script for automation (cron, CI/CD, etc.)

**Usage:**
```bash
# Basic usage
./auto-rebuild-settings.sh

# With auto-push enabled
./auto-rebuild-settings.sh --push

# With custom build target
./auto-rebuild-settings.sh --build-target Heltec_v3_companion_radio_ble
```

**What it does:**
1. Fetches from upstream
2. Updates main if needed
3. Rebases feature branch if needed
4. Builds firmware
5. Exits with appropriate status codes

## Workflow Options

### Option 1: Manual Rebuild (Recommended for testing)

Run `rebuild-with-settings.sh` whenever you want to:
- Test your changes against the latest upstream
- Build firmware with latest upstream + your changes
- Update your branch before pushing

```bash
cd MeshCore
./rebuild-with-settings.sh
```

### Option 2: Automated Rebuild (Cron/CI)

Set up a cron job or CI pipeline to automatically rebuild when upstream updates.

**Cron example (daily at 2 AM):**
```bash
0 2 * * * cd /path/to/MeshCore && ./auto-rebuild-settings.sh --push >> /var/log/meshcore-rebuild.log 2>&1
```

**GitHub Actions example:**
```yaml
name: Auto Rebuild Settings Screen
on:
  schedule:
    - cron: '0 2 * * *'  # Daily at 2 AM
  workflow_dispatch:  # Manual trigger

jobs:
  rebuild:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Setup PlatformIO
        uses: platformio/setup-platformio@v1
      - name: Rebuild with settings
        run: ./auto-rebuild-settings.sh --push
```

### Option 3: Integration with sync-all-repos.sh

Modify your `sync-all-repos.sh` to automatically rebuild after syncing MeshCore:

```bash
# At the end of sync_repo function, after syncing MeshCore:
if [ "$repo_dir" = "MeshCore" ] && [ "$was_updated" = true ]; then
    echo -e "  ${YELLOW}Triggering rebuild with settings screen...${NC}"
    cd "$repo_path"
    ./auto-rebuild-settings.sh --push
fi
```

## Git Workflow

### Branch Structure

```
upstream/main  ──────────────┐
                              │
origin/main    ───────────────┼───┐
                              │   │
                              │   └───> dev-settingsscreen (your changes)
                              │
                              └───> (other branches)
```

### Rebase vs Merge

This workflow uses **rebase** to keep a clean, linear history:
- ✅ Clean history (no merge commits)
- ✅ Easier to see your changes
- ✅ Easier to maintain

**If conflicts occur:**
1. Resolve conflicts manually
2. `git add <resolved-files>`
3. `git rebase --continue`
4. Re-run the build script

## Configuration

### Change Build Target

Edit the `BUILD_TARGET` variable in either script:
```bash
BUILD_TARGET="RAK_4631_companion_radio_ble"  # Change this
```

### Enable Auto-Push

In `rebuild-with-settings.sh`:
```bash
AUTO_PUSH=true  # Change from false
```

Or use `--push` flag with `auto-rebuild-settings.sh`:
```bash
./auto-rebuild-settings.sh --push
```

### Multiple Build Targets

Modify the build step to build multiple targets:
```bash
# In rebuild-with-settings.sh, replace the build step with:
for target in "RAK_4631_companion_radio_ble" "Heltec_v3_companion_radio_ble"; do
    echo -e "\n${YELLOW}Building $target...${NC}"
    ./build.sh build-firmware "$target"
done
```

## Troubleshooting

### Rebase Conflicts

If rebase fails with conflicts:
1. Resolve conflicts in the affected files
2. `git add <resolved-files>`
3. `git rebase --continue`
4. Re-run the build script

### Build Failures

If build fails:
1. Check PlatformIO environment configuration
2. Verify all dependencies are installed
3. Check for compilation errors in the output
4. Ensure you're on the correct branch

### Upstream Sync Issues

If upstream sync fails:
1. Check network connectivity
2. Verify upstream remote is configured: `git remote -v`
3. Check upstream repository access

## Best Practices

1. **Always test locally** before pushing
2. **Review changes** after rebase to ensure nothing broke
3. **Keep commits clean** - use `git rebase -i` to clean up if needed
4. **Tag releases** - tag your built firmware versions
5. **Document changes** - keep notes on what your branch adds/modifies

## Example Workflow

```bash
# 1. Daily sync (runs automatically via cron)
./sync-all-repos.sh

# 2. Manual rebuild when needed
cd MeshCore
./rebuild-with-settings.sh

# 3. Test the firmware
# Flash to device and test

# 4. If everything works, push (if AUTO_PUSH=false)
git push origin dev-settingsscreen --force-with-lease
```

## Integration with Existing Sync Script

To integrate with your existing `sync-all-repos.sh`, add this at the end of the `sync_repo` function:

```bash
# After syncing MeshCore, rebuild with settings screen
if [ "$repo_dir" = "MeshCore" ] && [ "$was_updated" = true ]; then
    echo -e "\n${BLUE}🔄 Triggering rebuild with settings screen...${NC}"
    if [ -f "$repo_path/auto-rebuild-settings.sh" ]; then
        cd "$repo_path"
        ./auto-rebuild-settings.sh --push
        cd "$BASE_DIR"
    fi
fi
```

