#!/usr/bin/env bash

# Script to update dev-cnfi branch with latest changes from main
# Run this whenever main gets updated from upstream

set -e

echo "=== Updating dev-cnfi with latest main ==="
echo ""

# Check if we're in a git repository
if ! git rev-parse --git-dir > /dev/null 2>&1; then
    echo "Error: Not in a git repository"
    exit 1
fi

# Check for uncommitted changes
if [ -n "$(git status --porcelain)" ]; then
    echo "Error: You have uncommitted changes. Please commit or stash them first."
    exit 1
fi

# Fetch latest
echo "1. Fetching latest from origin and upstream..."
git fetch origin
git fetch upstream 2>/dev/null || echo "   (upstream remote not found, using origin only)"

# Update main from upstream first (if upstream exists)
if git remote | grep -q upstream; then
    echo ""
    echo "1b. Syncing upstream/main to origin/main..."
    git checkout main
    UPSTREAM_COMMIT=$(git rev-parse upstream/main 2>/dev/null || echo "")
    ORIGIN_COMMIT=$(git rev-parse origin/main 2>/dev/null || echo "")
    
    if [ -n "$UPSTREAM_COMMIT" ] && [ "$UPSTREAM_COMMIT" != "$ORIGIN_COMMIT" ]; then
        echo "   Upstream has new commits, updating origin/main..."
        git merge upstream/main
        git push origin main
        echo "   ✓ origin/main synced with upstream"
    else
        echo "   ✓ origin/main is already up-to-date with upstream"
    fi
fi

# Update main
echo ""
echo "2. Updating local main branch..."
git checkout main
git pull origin main

# Check if main has new commits
MAIN_HEAD=$(git rev-parse main)
DEV_CNFI_BASE=$(git merge-base dev-cnfi main 2>/dev/null || echo "")

if [ "$MAIN_HEAD" = "$DEV_CNFI_BASE" ]; then
    echo ""
    echo "✓ dev-cnfi is already up-to-date with main"
    git checkout dev-cnfi
    exit 0
fi

echo ""
echo "3. New commits in main:"
git log --oneline ${DEV_CNFI_BASE}..main

# Update dev-cnfi
echo ""
echo "4. Updating dev-cnfi branch..."
git checkout dev-cnfi

# Try merge first (preserves history)
echo "   Merging main into dev-cnfi..."
if git merge main --no-edit; then
    echo ""
    echo "✓ Successfully merged main into dev-cnfi"
    echo ""
    echo "5. Current status:"
    git log --oneline --graph -10
    echo ""
    echo "Next steps:"
    echo "  - Review the merge: git log --oneline -5"
    echo "  - Test the build"
    echo "  - Push to origin: git push origin dev-cnfi"
else
    echo ""
    echo "⚠ Merge had conflicts. Resolve them and run:"
    echo "  git add ."
    echo "  git commit -m 'Resolve conflicts with upstream updates'"
    echo ""
    echo "Or abort with:"
    echo "  git merge --abort"
    exit 1
fi

echo ""
echo "=== Update complete ==="

