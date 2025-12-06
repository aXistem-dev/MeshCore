# Releasing Slunsecore Firmware

GitHub Actions is set up to automatically build and release Slunsecore firmware.

## Automatic Builds

Firmware is automatically built in two scenarios:

### 1. Branch Pushes

When code is pushed to the `slunsecore` branch, all firmware types are automatically built:
- Companion firmwares
- Repeater firmwares
- Room server firmwares

**Note**: These builds create artifacts but do NOT create GitHub Releases. Use this for testing builds.

### 2. Tag-Based Releases

When you push a release tag, the corresponding firmware type is built and a draft GitHub Release is created.

#### Tag Formats

Use the following tag formats for releases:

- `slunsecore-companion-v1.0.0` - Builds all companion firmwares
- `slunsecore-repeater-v1.0.0` - Builds all repeater firmwares
- `slunsecore-room-server-v1.0.0` - Builds all room server firmwares

> **NOTE**: Replace `v1.0.0` with the version you want to release. For detailed tag format specification, see [VERSIONING.md](VERSIONING.md#tag-naming-convention).

#### Release Process

1. **Create and push the tag**:
   ```bash
   git tag slunsecore-companion-v1.2.3
   git push origin slunsecore-companion-v1.2.3
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
git tag slunsecore-companion-v1.2.3
git tag slunsecore-repeater-v1.2.3
git tag slunsecore-room-server-v1.2.3
git push origin --tags
```

Each tag will trigger its own build and create its own draft release.

## Related Documentation

- **Version strings and filenames**: See [VERSIONING.md](VERSIONING.md)
- **Development workflow**: See [WORKFLOW.md](WORKFLOW.md)
