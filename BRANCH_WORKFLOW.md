# Branch Workflow Strategy for Settings Screen Customizations

## Current Situation
- **`main`**: Upstream/main branch at v1.11.0
- **`dev-settingsscreen`**: Feature branch based on v1.10.0 with 9 custom commits
- **`dev-cnfi`**: Currently at same point as main (v1.11.0)

## Recommended Workflow: Two-Branch Strategy

### Branch Purposes

1. **`dev-settingsscreen`** (Feature Branch)
   - Keep as-is: Historical record of your customizations
   - Based on v1.10.0
   - Never rebase or force-push (preserves history)
   - Use for reference and documentation

2. **`dev-cnfi`** (Deployment Branch)
   - Always based on latest `main` (currently v1.11.0)
   - Contains your customizations applied to latest upstream
   - This is what you build and deploy from
   - Can be rebased/updated whenever main updates

## Workflow Steps

### Initial Setup (One-time)

```bash
# 1. Ensure you're on dev-cnfi and it's up-to-date with main
git checkout dev-cnfi
git fetch origin
git reset --hard origin/main  # Make dev-cnfi match main exactly

# 2. Apply your customizations from dev-settingsscreen
# Option A: Cherry-pick commits (clean, preserves individual commits)
git cherry-pick 0beba2e^..2f53e23  # All commits from dev-settingsscreen

# Option B: Create a single merge commit (simpler, one commit)
git merge dev-settingsscreen --no-ff -m "Merge settings screen customizations from dev-settingsscreen"

# 3. Push dev-cnfi
git push origin dev-cnfi
```

### Ongoing Updates (When main gets updated)

```bash
# 1. Update main from upstream
git checkout main
git fetch upstream
git merge upstream/main  # or rebase if preferred
git push origin main

# 2. Update dev-cnfi with latest main
git checkout dev-cnfi
git merge main  # or: git rebase main (if you prefer linear history)

# 3. Resolve any conflicts (your customizations vs upstream changes)
# Fix conflicts, then:
git add .
git commit -m "Resolve conflicts with upstream v1.12.0"  # or continue rebase

# 4. Push dev-cnfi
git push origin dev-cnfi
```

## Alternative: Rebase Strategy

If you prefer a cleaner, linear history:

```bash
# 1. Create dev-cnfi from main
git checkout main
git checkout -b dev-cnfi
git push -u origin dev-cnfi

# 2. Rebase your commits onto main
git rebase --onto main 6d32193 dev-settingsscreen

# 3. This creates a new branch with your commits on top of main
# Then merge or cherry-pick into dev-cnfi
```

## Recommended Approach: Merge Strategy

**Why merge over rebase:**
- Preserves commit history
- Easier to track what came from where
- Less risk of losing work
- Can see the "merge point" clearly

**Workflow:**
1. Keep `dev-settingsscreen` as historical reference
2. Use `dev-cnfi` as your working deployment branch
3. When main updates: merge main into dev-cnfi
4. Resolve conflicts as needed
5. Build and deploy from dev-cnfi

## GitHub Actions Workflow Update

Update `.github/workflows/build-settingsscreen-releases.yml` to:
- Build from `dev-cnfi` branch instead of `dev-settingsscreen`
- Or keep building from `dev-settingsscreen` but base it on `dev-cnfi` for the actual code

## Benefits of This Approach

1. **Clean separation**: Feature branch vs deployment branch
2. **Easy updates**: Just merge main into dev-cnfi when upstream updates
3. **History preservation**: dev-settingsscreen remains as original work
4. **Flexibility**: Can always go back to dev-settingsscreen if needed
5. **No main pollution**: Never touches main branch

