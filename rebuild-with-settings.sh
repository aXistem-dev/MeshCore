#!/bin/bash

# Rebuild firmware with dev-settingsscreen changes on top of latest main
# This script:
# 1. Syncs upstream/main to origin/main
# 2. Rebases dev-settingsscreen onto origin/main
# 3. Builds the firmware
# 4. Optionally pushes the updated branch

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
MAIN_BRANCH="main"
FEATURE_BRANCH="dev-settingsscreen"
BUILD_TARGET="RAK_4631_companion_radio_ble"  # Change this to your target
AUTO_PUSH=false  # Set to true to automatically push after successful build

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

# Step 3: Rebase feature branch onto main
echo -e "\n${YELLOW}Step 3: Rebasing $FEATURE_BRANCH onto $MAIN_BRANCH...${NC}"
git checkout "$FEATURE_BRANCH" 2>&1 | grep -v "^$" || true

# Check if rebase is needed
FEATURE_BASE=$(git merge-base "$FEATURE_BRANCH" "$MAIN_BRANCH")
MAIN_HEAD=$(git rev-parse "$MAIN_BRANCH")

if [ "$FEATURE_BASE" != "$MAIN_HEAD" ]; then
    echo -e "  ${BLUE}Rebasing $FEATURE_BRANCH onto $MAIN_BRANCH...${NC}"
    if git rebase "$MAIN_BRANCH" 2>&1; then
        echo -e "${GREEN}  ✓ Rebase successful${NC}"
        
        # Push the rebased branch
        if [ "$AUTO_PUSH" = true ]; then
            echo -e "\n${YELLOW}Step 4: Pushing rebased $FEATURE_BRANCH...${NC}"
            git push origin "$FEATURE_BRANCH" --force-with-lease 2>&1 | grep -v "^$" || true
            echo -e "${GREEN}  ✓ Pushed to origin/$FEATURE_BRANCH${NC}"
        else
            echo -e "\n${YELLOW}Step 4: Rebase complete. Push manually with:${NC}"
            echo -e "  ${BLUE}git push origin $FEATURE_BRANCH --force-with-lease${NC}"
        fi
    else
        echo -e "${RED}  ❌ Rebase failed - conflicts need to be resolved manually${NC}"
        echo -e "${YELLOW}  Resolve conflicts, then run: git rebase --continue${NC}"
        exit 1
    fi
else
    echo -e "${GREEN}  ✓ $FEATURE_BRANCH is already based on latest $MAIN_BRANCH${NC}"
fi

# Step 4: Build firmware
echo -e "\n${YELLOW}Step 5: Building firmware for $BUILD_TARGET...${NC}"
if [ -f "build.sh" ]; then
    # Use existing build script
    FIRMWARE_VERSION="${FIRMWARE_VERSION:-v1.0.0}" ./build.sh build-firmware "$BUILD_TARGET"
else
    # Fallback to direct pio build
    pio run -e "$BUILD_TARGET"
fi

if [ $? -eq 0 ]; then
    echo -e "\n${GREEN}✓ Build successful!${NC}"
    echo -e "${BLUE}Firmware location:${NC}"
    echo -e "  ${BLUE}.pio/build/$BUILD_TARGET/firmware.hex${NC}"
    echo -e "  ${BLUE}.pio/build/$BUILD_TARGET/firmware.zip${NC}"
else
    echo -e "\n${RED}❌ Build failed${NC}"
    exit 1
fi

echo -e "\n${GREEN}=== Done ===${NC}"

