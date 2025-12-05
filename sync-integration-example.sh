#!/bin/bash
# Example integration snippet for sync-all-repos.sh
# Add this code to your sync-all-repos.sh after syncing MeshCore successfully

# Optional: Auto-update dev-cnfi with settings screen after sync
# Uncomment the following block to enable automatic updating

# if [ "$repo_dir" = "MeshCore" ] && [ "$was_updated" = true ]; then
#     echo -e "\n${BLUE}🔄 MeshCore was updated - updating dev-cnfi with settings screen...${NC}"
#     
#     # Check if update script exists
#     if [ -f "$repo_path/update-dev-cnfi.sh" ]; then
#         cd "$repo_path"
#         
#         # Run update (without auto-push to avoid accidental pushes)
#         if ./update-dev-cnfi.sh; then
#             echo -e "${GREEN}  ✓ dev-cnfi updated successfully${NC}"
#             echo -e "${YELLOW}  Push manually to trigger build: git push origin dev-cnfi${NC}"
#         else
#             echo -e "${RED}  ❌ Update failed - check logs${NC}"
#             # Don't fail the entire sync if update fails
#         fi
#         
#         cd "$BASE_DIR"
#     else
#         echo -e "${YELLOW}  ⚠️  Update script not found, skipping${NC}"
#     fi
# fi

