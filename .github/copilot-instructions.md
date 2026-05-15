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
3. **Stage Files**: Run `git add .` to stage all changes including CHANGELOG
4. **Generate Commit Message**: Create message using format:

   ```
   feat: [category] Short description of main change

   - Detailed bullet point 1
   - Detailed bullet point 2
   - Detailed bullet point 3

   Closes related issues if applicable
   ```

5. **Show Summary**: Display `git diff --cached --stat` to confirm staged changes

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

## General Development Notes

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
