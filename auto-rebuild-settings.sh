#!/bin/bash

# Automated rebuild script - updates dev-slunsecore with latest upstream and triggers GitHub Actions build
# Can be run via cron or manually
# Usage: ./auto-rebuild-settings.sh [--push] [--build-target TARGET]
#
# Note: This script updates dev-slunsecore branch. GitHub Actions automatically builds on push.

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Configuration
MAIN_BRANCH="main"
FEATURE_BRANCH="dev-slunsecore"
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

echo -e "${BLUE}=== Auto-Update dev-slunsecore with Latest Upstream ===${NC}\n"

if [ -f "update-dev-slunsecore.sh" ]; then
    echo -e "${YELLOW}Running update-dev-slunsecore.sh...${NC}"
    ./update-dev-slunsecore.sh

    if [ $? -eq 0 ]; then
        echo -e "\n${GREEN}✓ dev-slunsecore updated successfully${NC}"

        if [ "$AUTO_PUSH" = true ]; then
            echo -e "\n${YELLOW}Pushing dev-slunsecore to trigger GitHub Actions build...${NC}"
            git checkout "$FEATURE_BRANCH"
            git push origin "$FEATURE_BRANCH" 2>&1 | grep -v "^$" || true
            echo -e "${GREEN}  ✓ Pushed to origin - GitHub Actions will build automatically${NC}"
            echo -e "${BLUE}  Check build status: https://github.com/axistem-dev/MeshCore/actions${NC}"
        else
            echo -e "\n${YELLOW}To trigger build, push manually:${NC}"
            echo -e "  ${BLUE}git push origin $FEATURE_BRANCH${NC}"
        fi
        exit 0
    else
        echo -e "\n${RED}❌ Update failed${NC}"
        exit 1
    fi
else
    echo -e "${YELLOW}update-dev-slunsecore.sh not found - fetching and merging main into $FEATURE_BRANCH...${NC}"
    git fetch upstream "$MAIN_BRANCH" 2>&1 || true
    git checkout "$MAIN_BRANCH" 2>&1 || true
    git pull upstream "$MAIN_BRANCH" 2>&1 || true
    git push origin "$MAIN_BRANCH" 2>&1 || true
    git checkout "$FEATURE_BRANCH" 2>&1 || true
    if git merge "$MAIN_BRANCH" --no-edit 2>&1; then
        if [ "$AUTO_PUSH" = true ]; then
            git push origin "$FEATURE_BRANCH" 2>&1 || true
        fi
        exit 0
    else
        echo -e "${RED}❌ Merge failed${NC}"
        exit 1
    fi
fi
