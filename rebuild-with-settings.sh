#!/bin/bash

# Rebuild workflow: Updates dev-slunsecore with latest upstream and triggers GitHub Actions build
# This script:
# 1. Syncs upstream/main to origin/main
# 2. Merges main into dev-slunsecore (SlunseCore customizations)
# 3. Optionally pushes to trigger GitHub Actions build
#
# Note: GitHub Actions automatically builds settings screen firmwares on push to dev-slunsecore

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
MAIN_BRANCH="main"
FEATURE_BRANCH="dev-slunsecore"
BUILD_TARGET="RAK_4631_companion_radio_ble"
AUTO_PUSH=false

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo -e "${BLUE}=== Rebuild Firmware with Settings Screen ===${NC}\n"

# Step 1: Check if upstream has updates
echo -e "${YELLOW}Step 1: Checking for upstream updates...${NC}"
git fetch upstream "$MAIN_BRANCH" 2>&1 | grep -v "^$" || true

UPSTREAM_COMMIT=$(git rev-parse upstream/$MAIN_BRANCH)
ORIGIN_COMMIT=$(git rev-parse origin/$MAIN_BRANCH 2>/dev/null || echo "")

if [ -z "$ORIGIN_COMMIT" ] || [ "$UPSTREAM_COMMIT" != "$ORIGIN_COMMIT" ]; then
    echo -e "${GREEN}  ✓ Upstream has new commits${NC}"
    echo -e "  ${BLUE}Upstream: ${UPSTREAM_COMMIT:0:7}${NC}"
    if [ -n "$ORIGIN_COMMIT" ]; then
        echo -e "  ${BLUE}Origin:   ${ORIGIN_COMMIT:0:7}${NC}"
    fi
    
    # Step 2: Update origin/main from upstream
    echo -e "\n${YELLOW}Step 2: Updating origin/$MAIN_BRANCH from upstream...${NC}"
    git checkout "$MAIN_BRANCH" 2>&1 | grep -v "^$" || true
    git pull upstream "$MAIN_BRANCH" 2>&1 | grep -v "^$" || true
    git push origin "$MAIN_BRANCH" 2>&1 | grep -v "^$" || true
    echo -e "${GREEN}  ✓ origin/$MAIN_BRANCH updated${NC}"
else
    echo -e "${GREEN}  ✓ origin/$MAIN_BRANCH is already up to date${NC}"
fi

# Step 3: Update dev-slunsecore with latest main
echo -e "\n${YELLOW}Step 3: Updating $FEATURE_BRANCH with latest $MAIN_BRANCH...${NC}"
git checkout "$FEATURE_BRANCH" 2>&1 | grep -v "^$" || true

FEATURE_BASE=$(git merge-base "$FEATURE_BRANCH" "$MAIN_BRANCH" 2>/dev/null || echo "")
MAIN_HEAD=$(git rev-parse "$MAIN_BRANCH")

if [ "$FEATURE_BASE" != "$MAIN_HEAD" ]; then
    echo -e "  ${BLUE}Merging $MAIN_BRANCH into $FEATURE_BRANCH...${NC}"
    if git merge "$MAIN_BRANCH" --no-edit 2>&1; then
        echo -e "${GREEN}  ✓ Merge successful${NC}"

        if [ "$AUTO_PUSH" = true ]; then
            echo -e "\n${YELLOW}Step 4: Pushing $FEATURE_BRANCH to trigger GitHub Actions...${NC}"
            git push origin "$FEATURE_BRANCH" 2>&1 | grep -v "^$" || true
            echo -e "${GREEN}  ✓ Pushed to origin/$FEATURE_BRANCH${NC}"
            echo -e "${BLUE}  GitHub Actions will build settings screen firmwares${NC}"
            echo -e "${BLUE}  Check build: https://github.com/axistem-dev/MeshCore/actions${NC}"
        else
            echo -e "\n${YELLOW}Step 4: Merge complete. Push manually to trigger build:${NC}"
            echo -e "  ${BLUE}git push origin $FEATURE_BRANCH${NC}"
        fi
    else
        echo -e "${RED}  ❌ Merge failed - conflicts need to be resolved manually${NC}"
        echo -e "${YELLOW}  Resolve conflicts, then run:${NC}"
        echo -e "  ${BLUE}git add .${NC}"
        echo -e "  ${BLUE}git commit -m 'Resolve conflicts with upstream'${NC}"
        exit 1
    fi
else
    echo -e "${GREEN}  ✓ $FEATURE_BRANCH is already up-to-date with $MAIN_BRANCH${NC}"
fi

echo -e "\n${GREEN}=== Done ===${NC}"
echo -e "${BLUE}Note: Firmware builds are handled by GitHub Actions on push to dev-slunsecore${NC}"

