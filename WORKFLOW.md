# Slunsecore Development Workflow

## Overview

Slunsecore is a customized version of MeshCore. This document describes the development and release workflow for maintaining compatibility with MeshCore while adding slunsecore-specific features.

## Branch Structure

- **`main`** - Upstream MeshCore library (read-only, we don't push here)
- **`slunsecore`** - Official release branch (auto-builds firmware via GitHub Actions)
- **`dev-slunsecore`** - Development/nightly branch (accumulates features, no auto-build)
- **`feature/<feature-name>`** - Local feature development branches

## Development Workflow

### 1. Creating a New Feature

```bash
# Start from dev-slunsecore
git checkout dev-slunsecore
git pull origin dev-slunsecore

# Create feature branch
git checkout -b feature/<feature-name>

# Develop your feature...
# Make commits...

# When feature is complete, push to dev-slunsecore
git checkout dev-slunsecore
git merge feature/<feature-name>
git push origin dev-slunsecore

# Delete local feature branch (optional)
git branch -d feature/<feature-name>
```

### 2. Nightly Builds

- Features are merged into `dev-slunsecore` as they're completed
- `dev-slunsecore` serves as the integration branch for testing
- No automatic firmware builds are triggered for `dev-slunsecore`

## Release Workflow

### Handling New MeshCore Releases

When a new MeshCore release is available:

#### Step 1: Merge MeshCore Release into dev-slunsecore (Testing Phase)

```bash
# Switch to dev-slunsecore
git checkout dev-slunsecore
git pull origin dev-slunsecore

# Merge the new MeshCore release
git merge main  # or: git merge <release-tag>

# Resolve any conflicts
# Test thoroughly to ensure compatibility

# Push the updated dev-slunsecore
git push origin dev-slunsecore
```

**Important:** This step does NOT trigger automatic builds. It's the testing phase.

#### Step 2: Release to slunsecore (Production Build)

```bash
# Switch to slunsecore
git checkout slunsecore
git pull origin slunsecore

# Merge dev-slunsecore (contains MeshCore release + all slunsecore features)
git merge dev-slunsecore

# Resolve any remaining conflicts (should be minimal)
# Final testing

# Push to trigger GitHub Actions auto-build (creates artifacts)
git push origin slunsecore

# Create and push release tags (creates GitHub Releases)
# See RELEASE.md for tag formats and process
git tag slunsecore-companion-v1.x.x
git tag slunsecore-repeater-v1.x.x
git tag slunsecore-room-server-v1.x.x
git push origin --tags
```

**Important:** 
- Only merge `dev-slunsecore` → `slunsecore` (never merge `main` directly to `slunsecore`)
- This ensures only tested code triggers automatic firmware builds
- The merge from `dev-slunsecore` brings in both the MeshCore release and all slunsecore features

## Key Principles

1. **Compatibility First**: All changes must remain compatible with MeshCore main
2. **Test Before Build**: All code is tested in `dev-slunsecore` before triggering builds
3. **No Direct Main → Slunsecore**: Never merge `main` directly to `slunsecore` to avoid untested builds
4. **Feature Isolation**: Each feature is developed in its own branch
5. **Clean History**: Feature branches are merged into `dev-slunsecore`, then `dev-slunsecore` is merged into `slunsecore`

## Workflow Diagram

```
main (MeshCore upstream)
  │
  ├─→ dev-slunsecore (merge for testing)
  │     │
  │     ├─→ feature/settings
  │     ├─→ feature/other-feature
  │     └─→ ... (other features)
  │
  └─→ slunsecore (merge from dev-slunsecore only)
        │
        └─→ [GitHub Actions: Auto-build firmware]
```

## Release Checklist

When preparing a new slunsecore release:

- [ ] Merge latest MeshCore release into `dev-slunsecore`
- [ ] Resolve all conflicts in `dev-slunsecore`
- [ ] Test thoroughly in `dev-slunsecore`
- [ ] Merge `dev-slunsecore` → `slunsecore`
- [ ] Final testing in `slunsecore`
- [ ] Push `slunsecore` branch (triggers artifact builds for testing)
- [ ] Create and push release tags (see [RELEASE.md](RELEASE.md))
- [ ] Verify GitHub Releases were created successfully

## Tagging Releases

When you're ready to create a release after merging to `slunsecore`, you'll need to create and push release tags. See [RELEASE.md](RELEASE.md) for the complete release process.

**Note**: For tag format specifications and versioning details, see [VERSIONING.md](VERSIONING.md).

## Notes

- Always use release tags when merging MeshCore releases for clarity
- Keep feature branches focused on single features
- Test thoroughly in `dev-slunsecore` before releasing to `slunsecore`
- The workflow ensures `slunsecore` always contains tested, stable code
