#!/usr/bin/env bash

# Script to set up dev-cnfi branch with customizations from dev-settingsscreen
# This applies your settings screen changes to the latest main branch

set -e

echo "=== Setting up dev-cnfi branch ==="
echo ""

# Check if we're in a git repository
if ! git rev-parse --git-dir > /dev/null 2>&1; then
    echo "Error: Not in a git repository"
    exit 1
fi

# Fetch latest from origin
echo "1. Fetching latest from origin..."
git fetch origin

# Ensure we're starting from a clean state
if [ -n "$(git status --porcelain)" ]; then
    echo "Warning: You have uncommitted changes. Stashing them..."
    git stash
    STASHED=true
else
    STASHED=false
fi

# Checkout and update main
echo ""
echo "2. Updating main branch..."
git checkout main
git pull origin main

# Checkout or create dev-cnfi
echo ""
echo "3. Setting up dev-cnfi branch..."
if git show-ref --verify --quiet refs/heads/dev-cnfi; then
    echo "   dev-cnfi branch exists, resetting to main..."
    git checkout dev-cnfi
    git reset --hard origin/main
else
    echo "   Creating dev-cnfi branch from main..."
    git checkout -b dev-cnfi
fi

# Show what commits will be applied
echo ""
echo "4. Commits from dev-settingsscreen that will be applied:"
git log --oneline main..dev-settingsscreen

echo ""
read -p "5. Apply these commits to dev-cnfi? (y/n) " -n 1 -r
echo ""

if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo ""
    echo "6. Applying commits..."
    
    # Option 1: Cherry-pick (preserves individual commits)
    echo "   Using cherry-pick method (preserves commit history)..."
    FIRST_COMMIT=$(git log --reverse --oneline main..dev-settingsscreen | head -1 | cut -d' ' -f1)
    LAST_COMMIT=$(git log --oneline main..dev-settingsscreen | head -1 | cut -d' ' -f1)
    
    if git cherry-pick ${FIRST_COMMIT}^..${LAST_COMMIT}; then
        echo ""
        echo "✓ Successfully applied all commits!"
        echo ""
        echo "7. Current status:"
        git log --oneline -5
        echo ""
        echo "Next steps:"
        echo "  - Review the changes: git diff main"
        echo "  - Test the build"
        echo "  - Push to origin: git push origin dev-cnfi"
    else
        echo ""
        echo "⚠ Cherry-pick had conflicts. Resolve them and run:"
        echo "  git cherry-pick --continue"
        echo ""
        echo "Or abort with:"
        echo "  git cherry-pick --abort"
        exit 1
    fi
else
    echo "Aborted."
    git checkout dev-settingsscreen 2>/dev/null || true
fi

# Restore stashed changes if any
if [ "$STASHED" = true ]; then
    echo ""
    echo "Restoring stashed changes..."
    git stash pop
fi

echo ""
echo "=== Setup complete ==="

