#!/bin/bash
# Example integration snippet for sync-all-repos.sh
# Add this code after syncing MeshCore successfully.

# Optional: Auto-update dev-slunsecore with latest upstream after sync
# Uncomment the following block to enable automatic updating

# if [ "$repo_dir" = "MeshCore" ] && [ "$was_updated" = true ]; then
#     echo -e "\n${BLUE}🔄 MeshCore was updated - updating dev-slunsecore...${NC}"
#
#     if [ -f "$repo_path/update-dev-slunsecore.sh" ]; then
#         cd "$repo_path"
#
#         if ./update-dev-slunsecore.sh; then
#             echo -e "${GREEN}  ✓ dev-slunsecore updated successfully${NC}"
#             echo -e "${YELLOW}  Push manually to trigger build: git push origin dev-slunsecore${NC}"
#         else
#             echo -e "${RED}  ❌ Update failed - check logs${NC}"
#         fi
#
#         cd "$BASE_DIR"
#     else
#         echo -e "${YELLOW}  ⚠️  update-dev-slunsecore.sh not found, skipping${NC}"
#     fi
# fi

