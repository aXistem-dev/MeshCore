#!/bin/bash

# Automated rebuild script - updates dev-cnfi with latest upstream and triggers GitHub Actions build
# Can be run via cron or manually
# Usage: ./auto-rebuild-settings.sh [--push]
#
# Note: This script updates dev-cnfi branch. GitHub Actions automatically builds on push.

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Configuration
MAIN_BRANCH="main"
DEPLOYMENT_BRANCH="dev-cnfi"
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

echo -e "${BLUE}=== Auto-Update dev-cnfi with Latest Upstream ===${NC}\n"

# Use the update script which handles everything
if [ -f "update-dev-cnfi.sh" ]; then
    echo -e "${YELLOW}Running update-dev-cnfi.sh...${NC}"
    ./update-dev-cnfi.sh
    
    if [ $? -eq 0 ]; then
        echo -e "\n${GREEN}✓ dev-cnfi updated successfully${NC}"
        
        if [ "$AUTO_PUSH" = true ]; then
            echo -e "\n${YELLOW}Pushing dev-cnfi to trigger GitHub Actions build...${NC}"
            git checkout "$DEPLOYMENT_BRANCH"
            git push origin "$DEPLOYMENT_BRANCH" 2>&1 | grep -v "^$" || true
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
    echo -e "${RED}Error: update-dev-cnfi.sh not found${NC}"
    exit 1
fi
