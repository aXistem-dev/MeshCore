# How Your Customizations Are Preserved

## ✅ Yes, Your Customizations Are Always Preserved

Your settings screen customizations **will always remain** in `dev-cnfi`, even when upstream updates to new versions (1.12.0, 1.13.0, etc.).

## How It Works

### Your Customizations Are Commits

Your customizations exist as **commits** in `dev-cnfi`:
- `f9425f2` - Add settings screen with screen always on, brightness control, and share pos in adverts
- `fac2925` - Add timezone and clock screensaver settings to the UI
- `230fab9` - Add automated rebuild scripts and documentation
- `130c64e` - Enhance firmware renaming and preference loading logic
- `80e4a6c` - Restored bluetooth logic
- `67aaadc` - Reverted SSD1306Display class
- `ad378a5` - Update workflow to build from dev-cnfi branch

These commits **modify specific files** that are **not part of upstream**:
- `examples/companion_radio/ui-new/UITask.cpp` - Your settings screen code
- `examples/companion_radio/NodePrefs.h` - Your new preference fields
- `examples/companion_radio/DataStore.cpp` - Your preference loading logic
- `.github/workflows/build-settingsscreen-releases.yml` - Your build workflow

### What Happens When Upstream Updates

When upstream releases v1.12.0:

1. **You sync upstream → origin/main**
   - Upstream's new code goes into `origin/main`
   - Upstream's changes are in **different files** (usually)

2. **You merge main → dev-cnfi**
   - Git applies upstream's changes to `dev-cnfi`
   - **Your commits remain on top**
   - Your customizations are **preserved**

3. **Result**
   - `dev-cnfi` = Latest upstream (v1.12.0) + Your customizations
   - Your settings screen code is still there
   - Your preference fields are still there
   - Everything works together

## Example Scenario

### Before Upstream Update (v1.11.0)
```
main (v1.11.0)
  └── dev-cnfi
       ├── Upstream v1.11.0 code
       └── Your customizations (7 commits)
```

### After Upstream Update (v1.12.0)
```
main (v1.12.0) ← Updated from upstream
  └── dev-cnfi
       ├── Upstream v1.12.0 code ← Merged from main
       └── Your customizations (7 commits) ← Still there!
```

## When Conflicts Occur

Conflicts only happen if:
- Upstream modifies the **same files** you modified
- Upstream changes the **same lines** you changed

### Your Customized Files (Low Conflict Risk)

These files are **unlikely** to conflict because upstream doesn't modify them:

1. **`examples/companion_radio/ui-new/UITask.cpp`**
   - Your settings screen code
   - Upstream rarely touches UI code
   - **Low conflict risk**

2. **`examples/companion_radio/NodePrefs.h`**
   - Your new preference fields
   - Upstream doesn't add these fields
   - **Very low conflict risk**

3. **`examples/companion_radio/DataStore.cpp`**
   - Your preference loading logic
   - Upstream might modify this, but your changes are additive
   - **Medium conflict risk** (but easy to resolve)

4. **`.github/workflows/build-settingsscreen-releases.yml`**
   - Your build workflow
   - Upstream doesn't have this file
   - **Zero conflict risk**

### If Conflicts Do Occur

If upstream modifies a file you also modified:

1. **Git marks the conflict**
   ```bash
   Auto-merging examples/companion_radio/DataStore.cpp
   CONFLICT (content): Merge conflict in examples/companion_radio/DataStore.cpp
   ```

2. **You resolve it manually**
   - Open the conflicted file
   - Keep your customizations
   - Integrate upstream's changes
   - Commit the resolution

3. **Your customizations remain**
   - You explicitly keep your code
   - Upstream's changes are integrated
   - Everything works together

## Real-World Example

### Scenario: Upstream updates DataStore.cpp

**Upstream's change:**
```cpp
// Upstream adds new field
uint8_t new_upstream_field;
```

**Your change:**
```cpp
// You added these fields
uint8_t screen_always_on;
uint8_t screen_brightness;
uint8_t screen_screensaver;
int16_t timezone_offset_minutes;
```

**After merge:**
```cpp
// Both are preserved!
uint8_t new_upstream_field;  // From upstream
uint8_t screen_always_on;    // Your customization
uint8_t screen_brightness;    // Your customization
uint8_t screen_screensaver;   // Your customization
int16_t timezone_offset_minutes;  // Your customization
```

**Result**: Both sets of changes coexist. No conflict!

## Guarantees

✅ **Your customizations are preserved** - They're commits in `dev-cnfi`  
✅ **Upstream updates are integrated** - Latest code is merged in  
✅ **Both coexist** - Your code + upstream code work together  
✅ **Version updates work** - When upstream goes to 1.12.0, you get 1.12.0 + your customizations  

## Verification

You can always verify your customizations are present:

```bash
# Check what's unique to dev-cnfi
git log --oneline main..dev-cnfi

# Check what files you modified
git diff main dev-cnfi --name-only

# See your actual changes
git diff main dev-cnfi
```

## Summary

**Your customizations are safe because:**
1. They're in separate commits
2. They modify files upstream rarely touches
3. Git merge preserves both sets of changes
4. Conflicts are rare and easy to resolve

**When upstream updates:**
- Your code stays in `dev-cnfi`
- Upstream's code gets merged in
- Both work together
- You get: Latest upstream + Your customizations

