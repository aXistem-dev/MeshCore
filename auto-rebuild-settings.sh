#!/bin/bash

# Automated rebuild script - checks for updates and rebuilds if needed
# Can be run via cron or manually
# Usage: ./auto-rebuild-settings.sh [--push] [--build-target TARGET]

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Configuration
MAIN_BRANCH="main"
FEATURE_BRANCH="dev-settingsscreen"
BUILD_TARGET="RAK_4631_companion_radio_ble"
AUTO_PUSH=false

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --push)
            AUTO_PUSH=true
            shift
            ;;
        --build-target)
            BUILD_TARGET="$2"
            shift 2
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo -e "${BLUE}=== Auto-Rebuild with Settings Screen ===${NC}\n"

# Fetch from upstream
echo -e "${YELLOW}Fetching from upstream...${NC}"
git fetch upstream "$MAIN_BRANCH" 2>&1 | grep -v "^$" || true

# Check if main needs updating
UPSTREAM_COMMIT=$(git rev-parse upstream/$MAIN_BRANCH)
ORIGIN_MAIN_COMMIT=$(git rev-parse origin/$MAIN_BRANCH 2>/dev/null || echo "")
NEEDS_UPDATE=false

if [ -z "$ORIGIN_MAIN_COMMIT" ] || [ "$UPSTREAM_COMMIT" != "$ORIGIN_MAIN_COMMIT" ]; then
    NEEDS_UPDATE=true
    echo -e "${GREEN}  ✓ New commits detected in upstream/$MAIN_BRANCH${NC}"
else
    echo -e "${GREEN}  ✓ origin/$MAIN_BRANCH is up to date${NC}"
fi

# Check if feature branch needs rebasing
if [ "$NEEDS_UPDATE" = true ]; then
    # Update main first
    git checkout "$MAIN_BRANCH" 2>&1 | grep -v "^$" || true
    git pull upstream "$MAIN_BRANCH" 2>&1 | grep -v "^$" || true
    git push origin "$MAIN_BRANCH" 2>&1 | grep -v "^$" || true
    
    # Check if feature branch needs rebase
    git checkout "$FEATURE_BRANCH" 2>&1 | grep -v "^$" || true
    FEATURE_BASE=$(git merge-base "$FEATURE_BRANCH" "$MAIN_BRANCH")
    MAIN_HEAD=$(git rev-parse "$MAIN_BRANCH")
    
    if [ "$FEATURE_BASE" != "$MAIN_HEAD" ]; then
        echo -e "\n${YELLOW}Rebasing $FEATURE_BRANCH onto $MAIN_BRANCH...${NC}"
        if git rebase "$MAIN_BRANCH" 2>&1; then
            echo -e "${GREEN}  ✓ Rebase successful${NC}"
            
            if [ "$AUTO_PUSH" = true ]; then
                git push origin "$FEATURE_BRANCH" --force-with-lease 2>&1 | grep -v "^$" || true
                echo -e "${GREEN}  ✓ Pushed to origin${NC}"
            fi
        else
            echo -e "${RED}  ❌ Rebase failed - manual intervention needed${NC}"
            exit 1
        fi
    fi
fi

# Build firmware
echo -e "\n${YELLOW}Building firmware for $BUILD_TARGET...${NC}"
if [ -f "build.sh" ]; then
    FIRMWARE_VERSION="${FIRMWARE_VERSION:-v1.0.0}" ./build.sh build-firmware "$BUILD_TARGET"
else
    pio run -e "$BUILD_TARGET"
fi

if [ $? -eq 0 ]; then
    echo -e "\n${GREEN}✓ Build successful!${NC}"
    exit 0
else
    echo -e "\n${RED}❌ Build failed${NC}"
    exit 1
fi
