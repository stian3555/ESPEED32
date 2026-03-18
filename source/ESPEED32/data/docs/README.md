# ESPEED32 On-Device Docs (SPIFFS)

This folder is the single source of truth for user documentation that is:

- versioned in Git, and
- served from the controller at `/docs`.

## Why this path

Keep docs under `source/ESPEED32/data/docs/`.

Reason: Arduino/ESP32 filesystem tooling expects the `data/` folder next to the sketch.
By keeping one canonical location, docs in Git and docs on device stay aligned.

## Structure

- `en/index.html` - full English user guide
- `no/index.html` - full Norwegian user guide
- `es/index.html` - full Spanish user guide
- `de/index.html` - full German user guide
- `it/index.html` - full Italian user guide
- `assets/curve_examples.svg` - curve graph from manuals
- `assets/advanced_brake_examples.svg` - Alt.Brake, QUICK, and DRAG concept graphs
- `assets/fade_examples.svg` - fade off/on graph examples
- `assets/fade_curve_examples.svg` - combined fade and curve graph examples
- `assets/limit_examples.svg` - limit and duty cap examples
- `assets/pwm_frequency_partial_examples.svg` - 1 kHz vs 5 kHz at 50% duty under LIMIT 100
- `assets/pwm_frequency_capped_examples.svg` - 1 kHz vs 5 kHz at 70% duty under LIMIT 70
- `assets/pwm_frequency_examples.svg` - PWM frequency examples
- `assets/wifi_examples.svg` - controller OLED WiFi info and full-screen QR examples
- `assets/display_modes.svg` - OLED list and grid view mockups
- `assets/advanced_config_editor_car_params.png` - wide browser screenshot of the Car Params editor
- `assets/advanced_config_editor_car_params_tall.png` - taller browser screenshot of the Car Params editor
- `assets/trig_cal.png` - trigger calibration figure

Related SPIFFS content (outside this folder):

- `../ui/index.html` - backup/restore web UI (single source, served at `/`)

## Update workflow

1. Edit HTML files in this folder.
2. Commit to Git.
3. If firmware and docs changed, run a full flash:
   - VS Code task: `ESPEED32: Flash firmware + SPIFFS`
   - or terminal: `./scripts/flash_all.sh`
4. Upload SPIFFS image only (when firmware is unchanged):
   - VS Code task: `ESPEED32: Upload SPIFFS docs`
   - or terminal: `./scripts/upload_spiffs.sh`
5. Open `http://192.168.4.1/docs` while the controller is in WiFi mode.

## Runtime routes

Firmware serves docs from:

- `/docs` (language-aware default)
- `/docs/en`
- `/docs/no`
- `/docs/es`
- `/docs/de`
- `/docs/it`

Routes are implemented in `source/ESPEED32/connectivity_portal.cpp`.

## Optional GitHub Pages hosting

You can host these docs online (for example on GitHub Pages) without duplicating content.

Recommended approach:

1. Keep `source/ESPEED32/data/docs/` as the single source of truth.
2. Publish those files to your Pages site via CI.
3. Optionally publish `source/ESPEED32/data/ui/index.html` online too.

Repository workflow:

- `.github/workflows/pages.yml` publishes:
  - `/docs/` from `source/ESPEED32/data/docs/`
  - `/ui/` from `source/ESPEED32/data/ui/`
  - `/` (landing) from `source/ESPEED32/data/ui/index.html`
- Triggered automatically on push to `main` when docs/UI files change.

Notes for online UI publishing:

- The same `ui/index.html` can run in two modes:
  - device mode (served by controller): full WiFi + OTA + local API support
  - hosted mode (served from internet): USB-first mode, WiFi/OTA hidden
- Hosted mode is auto-detected by hostname and can be overridden with query:
  - `?mode=hosted`
  - `?mode=device`

Custom domain:

- Add a `.github/CNAME` file (example: `docs.yourdomain.com`).
- The workflow copies it to the published artifact automatically.
