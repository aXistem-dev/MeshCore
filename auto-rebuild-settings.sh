#!/bin/bash

# Automated rebuild script - updates dev-slunsecore with latest upstream and triggers GitHub Actions build
# Can be run via cron or manually
# Usage: ./auto-rebuild-settings.sh [--push]
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
DEPLOYMENT_BRANCH="dev-slunsecore"
AUTO_PUSH=false

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --push)
            AUTO_PUSH=true
            shift
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

if [ -f "rebuild-with-settings.sh" ]; then
    echo -e "${YELLOW}Running rebuild-with-settings.sh...${NC}"
    if [ "$AUTO_PUSH" = true ]; then
        AUTO_PUSH=true ./rebuild-with-settings.sh
    else
        ./rebuild-with-settings.sh
    fi

    if [ $? -eq 0 ]; then
        echo -e "\n${GREEN}✓ dev-slunsecore updated successfully${NC}"

        if [ "$AUTO_PUSH" = true ]; then
            echo -e "${GREEN}  ✓ Pushed to origin - GitHub Actions will build automatically${NC}"
            echo -e "${BLUE}  Check build status: https://github.com/axistem-dev/MeshCore/actions${NC}"
        else
            echo -e "\n${YELLOW}To trigger build, push manually:${NC}"
            echo -e "  ${BLUE}git push origin $DEPLOYMENT_BRANCH${NC}"
        fi
        exit 0
    else
        echo -e "\n${RED}❌ Update failed${NC}"
        exit 1
    fi
else
    echo -e "${RED}Error: rebuild-with-settings.sh not found${NC}"
    exit 1
fi
