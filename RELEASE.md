# Releasing Slunsecore Firmware

GitHub Actions is set up to automatically build and release Slunsecore firmware.

## Build Types

Firmware can be built in three ways:

### 1. Automatic Branch Pushes

When code is pushed to the `slunsecore` branch, all firmware types are automatically built:
- Companion firmwares
- Repeater firmwares
- Room server firmwares

**Note**: 
- These builds create artifacts but do NOT create GitHub Releases
- Builds from `slunsecore` branch use standard filenames (no `nightly-` prefix)
- Use this for testing builds before creating release tags

### 2. Manual Nightly Builds

You can manually trigger builds from any branch (typically `dev-slunsecore`) for nightly/testing builds:

1. Go to **Actions** tab in GitHub
2. Select the workflow you want to run (e.g., "Build Companion Firmwares")
3. Click **Run workflow**
4. Select the branch (defaults to `dev-slunsecore`)
5. Click **Run workflow**

**Note**: 
- Manual builds create artifacts but do NOT create GitHub Releases
- Perfect for testing builds from `dev-slunsecore` before merging to `slunsecore`
- Available branches: `dev-slunsecore`, `slunsecore`
- **Nightly builds from `dev-slunsecore` have `nightly-` prefix in filenames** to distinguish from official releases
  - Example: `nightly-RAK_4631_companion_radio_usb-v1.2.3-scdev-a1b2c3d.bin`

### 3. Tag-Based Releases

When you push a release tag, the corresponding firmware type is built and a draft GitHub Release is created.

#### Tag Formats

Use the following tag formats for releases:

- `sc-companion-v1.0.0` - Builds all companion firmwares
- `sc-repeater-v1.0.0` - Builds all repeater firmwares
- `sc-room-server-v1.0.0` - Builds all room server firmwares

> **NOTE**: Replace `v1.0.0` with the version you want to release. For detailed tag format specification, see [VERSIONING.md](VERSIONING.md#tag-naming-convention).

#### Release Process

1. **Create and push the tag**:
   ```bash
   git tag sc-companion-v1.2.3
   git push origin sc-companion-v1.2.3
   ```

2. **GitHub Actions automatically**:
   - Extracts the version from the tag (see [VERSIONING.md](VERSIONING.md#version-extraction) for details)
   - Builds all firmwares for that type
   - Creates a draft GitHub Release
   - Uploads firmware files to the release

3. **Finalize the release**:
   - Go to GitHub Releases page
   - Edit the draft release
   - Add release notes
   - Publish the release

#### Multiple Tags

You can push multiple tags on the same commit, and they will all build separately:
```bash
git tag sc-companion-v1.2.3
git tag sc-repeater-v1.2.3
git tag sc-room-server-v1.2.3
git push origin --tags
```

Each tag will trigger its own build and create its own draft release.

## Related Documentation

- **Version strings and filenames**: See [VERSIONING.md](VERSIONING.md)
- **Development workflow**: See [WORKFLOW.md](WORKFLOW.md)
