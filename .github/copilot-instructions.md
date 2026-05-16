# dtuGateway Copilot Instructions

## Prepare for Commit Workflow

When preparing to commit changes, automatically:

1. Review uncommitted changes via `git status`
2. Update [CHANGELOG.md](../../CHANGELOG.md) with user-facing changes
3. Stage all modified files with `git add`
4. Generate a suggested commit message
5. Display staged changes summary

### Trigger Commands

- "prepare for commit"
- "prepare commit"
- "prep commit"
- "prepare for commit with changelog: [description]"

### Workflow Steps

1. **Get Current Changes**: Run `git status --short` to list all modified files
2. **Update CHANGELOG**: Add entry under `[Unreleased]` section with:
   - Category (Added/Changed/Fixed based on keywords in description)
   - User description or inferred summary
   - Timestamp
3. **Branch-Specific Actions**:
   - **If on `develop` branch**: Also update [.snapshot-summary.md](../../.snapshot-summary.md):
     - **Automatically analyze** code changes and create user-centric summaries (no developer prompts)
     - **Infer category** based on change type:
       - ✨ Features: New capabilities or functionality added
       - 🔧 Improvements: Enhancements to existing features
       - 🐛 Fixes: Bug fixes and corrections
     - **Generate entry** as one-liner (max ~80 chars), user-focused (NO technical jargon)
     - **Add to appropriate section** in .snapshot-summary.md
     - **Present for review**: Show updated summary to developer with message like: "I've added these changes to the snapshot summary — review and adjust if needed"
     - Developer accepts/modifies/removes entries before final commit
     - Stage updated .snapshot-summary.md with CHANGELOG
   - **If on `main` or other branches**: Skip snapshot summary
4. **Stage Files**: Run `git add .` to stage all changes including CHANGELOG and snapshot summary
5. **Generate Commit Message**: Create message using format:

   ```
   feat: [category] Short description of main change

   - Detailed bullet point 1
   - Detailed bullet point 2
   - Detailed bullet point 3

   Closes related issues if applicable
   ```

6. **Show Summary**: Display `git diff --cached --stat` to confirm staged changes

### Project Context

**Main Technologies**:

- Firmware: ESP32 (PlatformIO, C++ Arduino)
- Display: TFT_eSPI library, GC9A01 240×240 OLED LCD
- Web Dashboard: Alpine.js 3.x (PROGMEM served)
- Backend: ArduinoJson, MQTT via PubSubClient
- Build System: Python scripts for web header generation

**Key Directories**:

- `src/` — Firmware source (main .ino + implementation)
- `include/` — Headers + PROGMEM web assets
- `web_dev/src/` — Dashboard source (HTML, CSS, JS)
- `web_dev/build/` — Header generation scripts
- `doc/` — Documentation and screenshots

**Current Focus Areas**:

- Battery + Solar+Battery monitor modes
- TFT display rendering for remote monitoring
- Alpine.js UI for mode selection and control
- MQTT data integration

## Division of Work: Copilot vs GitHub Workflow

This snapshot release system uses **purposeful separation of responsibilities** to ensure both user-friendly release notes and reliable automation:

### Copilot's Job (BEFORE Commit on develop)

**Goal**: Create human-curated, user-centric summaries automatically

**When developer says "prepare for commit" on develop branch:**

1. Copilot reads actual code changes (git diff)
2. Copilot analyzes each change for user impact
3. Copilot **automatically generates** user-centric summary entries
4. Copilot adds to `.snapshot-summary.md`:
   - Categorizes as ✨ Features, 🔧 Improvements, or 🐛 Fixes
   - Writes one-liner (max 80 chars) in USER-FOCUSED language
   - NO technical jargon
5. Copilot presents summary to developer: _"I've added these changes to snapshot — review and adjust if needed"_
6. Developer accepts, modifies, or removes entries
7. Both CHANGELOG and `.snapshot-summary.md` staged together

**Example workflow:**

- Code change: Removed jQuery library from dashboard
- Copilot generates: `"Dashboard Faster & Offline — Removed jQuery CDN dependency"`
- Category: 🔧 Improvements
- Developer reviews and accepts ✓

**Result**: `.snapshot-summary.md` contains well-curated, user-friendly descriptions (developer-approved, not developer-created)

### Workflow's Job (DURING Push to GitHub)

**Goal**: Combine summaries with technical details; auto-detect for genericity

**When commit is pushed to develop branch (dev_build.yml triggers):**

1. **Read static summaries** from `.snapshot-summary.md` (already prepared by Copilot)
2. **Auto-detect latest main tag** (e.g., `v2.3.0018`) using: `git tag --list "v*" --sort=-version:refname --merged main`
3. **Get commit history** since that tag
4. **Combine both**:
   - User summaries (readable, curated by Copilot - PRIMARY)
   - Commit log (technical details, for developers - OPTIONAL, only if main tag found)
5. **Generate snapshot release notes** at GitHub

**Result**: Snapshot users see = User summaries (always) + Commit history (optional, if found)

**Important**: Release notes are ALWAYS user-centric. Commit history is omitted if main release tags can't be found (no error messages, no fallback technical content).

### Why This Distribution Works

| Responsibility                | Copilot                             | Workflow                  |
| ----------------------------- | ----------------------------------- | ------------------------- |
| **User perspective**          | ✓ Creative, reads code              | ✗ Not needed              |
| **Readability**               | ✓ Auto-generates, developer refines | ✗ Static reading          |
| **Technical accuracy**        | ✗ Risky to guess                    | ✓ Queries git directly    |
| **Genericity (future-proof)** | ✗ Hardcoding versions               | ✓ Auto-detects tags       |
| **No LLM access needed**      | ✓ Runs at commit time               | ✓ No AI in GitHub Actions |

**Key insight**: Copilot's job is CREATIVE (summarizing code changes). Workflow's job is MECHANICAL (combining, auto-detecting). This split ensures:

- 🎯 Release notes are readable and user-focused
- 🔒 Automation is reliable and works for any future release
- 🚀 No manual intervention needed after each main release

## General Development Notes

### Snapshot Release Summary Maintenance

The [.snapshot-summary.md](../../.snapshot-summary.md) file is **automatically updated** when you run "prepare for commit" on the develop branch. It serves as the source for snapshot release notes shown on GitHub releases.

**Guidelines for Summary Entries**:

- ✨ **Features**: New capabilities (one-liner, max ~80 chars)
  - Good: "Firmware Update Progress UI — real-time feedback with timeout protection"
  - Bad: "impl firmware update with progress modal containing animated progress bar and 60s timeout and countdown display and auto-reload logic"

- 🔧 **Improvements**: Enhancements to existing features
  - Good: "Fixed /doupdate endpoint — now returns HTTP 200 OK"
  - Bad: "Modified POST handler return codes from 501 to 200 for RFC compliance"

- 🐛 **Fixes**: Bug fixes and corrections
  - Good: "Modal no longer reopens after page reload"
  - Bad: "Fixed issue where modal state was not properly cleared in localStorage"

**What the Copilot Agent Will Do**:

- Detect user-visible changes in git diff (skip version.h, version.json, build files)
- For each significant change, **automatically generate** a user-centric one-liner (max 80 chars)
- Categorize as ✨ Features, 🔧 Improvements, or 🐛 Fixes
- Append to appropriate section in `.snapshot-summary.md`
- **Present for review**: Show developer with message: "I've added these changes to the snapshot summary — review and adjust if needed"
- Developer accepts, modifies, or removes entries
- Stage the updated summary file with CHANGELOG

**How it's Used**:

1. Developer commits to `develop` with "prepare for commit"
2. Copilot updates `.snapshot-summary.md` automatically
3. Next push to develop triggers `dev_build.yml`
4. Workflow reads `.snapshot-summary.md` to generate snapshot release notes
5. Users see snapshot showing only changes since last **v*.*.\*** release tag

### Important: Snapshot Version Comparison Reference

**Good News**: The `dev_build.yml` workflow **automatically detects** the latest main release tag. You don't need to manually update `.snapshot-summary.md` after each main release.

**What This Means**:

- After merging develop → main and creating a new release tag (v2.4.46, v2.5.0, etc.)
- The next develop build will **automatically** compare against that new tag
- Release notes will show only NEW changes since the latest main release
- No manual intervention required ✓

**⚠️ IMPORTANT: .snapshot-summary.md Version Reference**

The `.snapshot-summary.md` file contains a **reference** to help developers understand context, but is **NOT used** for tag comparison:

- Used for: User-facing summaries in release notes
- NOT used for: Determining which commits to show (workflow auto-detects)

**When .snapshot-summary.md is OUT OF DATE** (e.g., still says v2.3.0018 but main is v2.4.46):

- ✅ Release notes are STILL CORRECT (workflow auto-detects v2.4.46)
- ⚠️ But the file is confusing for developers
- **Recommended**: Update .snapshot-summary.md reference after new main release to keep context fresh

**How to Update Version Reference** (optional, for clarity):

```bash
# Find the latest release tag on main branch
git tag --list "v*" --sort=-version:refname --merged main | head -1

# Verify the date of that release
git log -1 --format="%ai" <tag-name>

# Update .snapshot-summary.md header
# Example: Change "v2.3.0018 (September 13, 2025)" to "v2.4.46 (May 17, 2026)"
```

**Example Timeline**:

- Sept 13, 2025: Main release v2.3.0018, snapshot references it ✓
- May 16, 2026: Develop has many changes, .snapshot-summary.md still shows v2.3.0018
- May 17, 2026: New main release v2.4.46 created
- May 18, 2026: Dev commits to develop
  - ✅ Snapshot automatically compares to v2.4.46 (not v2.3.0018)
  - .snapshot-summary.md could still say v2.3.0018 (but should be updated for clarity)
  - Developer updates .snapshot-summary.md header → now shows v2.4.46
  - Future developers understand context ✓

**Key Takeaway**: You don't NEED to manually update the version reference, but SHOULD update it occasionally for documentation clarity.

### Version Management

- Version stored in [include/version.h](../../include/version.h)
- Build increments automatically (version_inc.py)
- Format: `MAJOR.MINOR.PATCH_variant` (e.g., 2.4.35_localDev)

### Web Dev Workflow

1. Edit source in `web_dev/src/` (index.html, style.css, app.js)
2. Run `python web_dev/build/build_headers.py` to minify and generate C headers
3. Commit headers to git (they're PROGMEM assets, not user files)
4. Test with local dev server: `python web_dev/mock_server/app.py`

### Display Testing

- Remote dashboard at `/index2.html` (Alpine.js v2)
- Test locally: http://127.0.0.1:5000/
- Device: http://[device-ip]/index2.html
