#!/bin/bash
# Example integration snippet for sync-all-repos.sh
# Add this code to your sync-all-repos.sh after line 220 (after "echo -e "  ${GREEN}✓ $repo_dir synced successfully${NC}"")

# Optional: Auto-rebuild MeshCore with settings screen after sync
# Uncomment the following block to enable automatic rebuilding

# if [ "$repo_dir" = "MeshCore" ] && [ "$was_updated" = true ]; then
#     echo -e "\n${BLUE}🔄 MeshCore was updated - triggering rebuild with settings screen...${NC}"
#     
#     # Check if rebuild script exists
#     if [ -f "$repo_path/auto-rebuild-settings.sh" ]; then
#         cd "$repo_path"
#         
#         # Run rebuild (without auto-push to avoid accidental pushes)
#         if ./auto-rebuild-settings.sh; then
#             echo -e "${GREEN}  ✓ Rebuild successful${NC}"
#         else
#             echo -e "${RED}  ❌ Rebuild failed - check logs${NC}"
#             # Don't fail the entire sync if rebuild fails
#         fi
#         
#         cd "$BASE_DIR"
#     else
#         echo -e "${YELLOW}  ⚠️  Rebuild script not found, skipping${NC}"
#     fi
# fi

