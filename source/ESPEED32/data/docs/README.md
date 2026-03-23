# ESPEED32 On-Device Docs (SPIFFS)

This folder contains the on-device documentation that is:

- versioned in Git, and
- served from the controller at `/docs`.

Generated note:

- `en/index.html`, `no/index.html`, `nl/index.html`, `pt/index.html`, `es/index.html`, `de/index.html`, and `it/index.html`
  are generated from `source/ESPEED32/docs_src/` via `scripts/build_docs.py`.
- These generated HTML files are not tracked in Git.

## Why this path

Keep published docs output under `source/ESPEED32/data/docs/`.

Reason: Arduino/ESP32 filesystem tooling expects the `data/` folder next to the sketch.
That keeps the generated output aligned with what is uploaded to the device.

## Structure

- `en/index.html` - generated English user guide
- `no/index.html` - generated Norwegian user guide
- `nl/index.html` - generated Dutch user guide
- `pt/index.html` - generated Portuguese user guide
- `es/index.html` - generated Spanish user guide
- `de/index.html` - generated German user guide
- `it/index.html` - generated Italian user guide
- `assets/curve_examples.svg` - curve graph from manuals
- `assets/adv_brake.svg` - Alt.Brake, QUICK, and DRAG concept graphs
- `assets/fade_examples.svg` - fade off/on graph examples
- `assets/fade_curve.svg` - combined fade and curve graph examples
- `assets/limit_examples.svg` - limit and duty cap examples
- `assets/pwm_part.svg` - 1 kHz vs 5 kHz at 50% duty under LIMIT 100
- `assets/pwm_cap.svg` - 1 kHz vs 5 kHz at 70% duty under LIMIT 70
- `assets/pwm_freq.svg` - PWM frequency examples
- `assets/wifi_examples.svg` - controller OLED WiFi info and full-screen QR examples
- `assets/display_modes.svg` - OLED list and grid view mockups
- `assets/car_params.png` - browser screenshot of the Car Params editor
- `assets/trig_cal.png` - trigger calibration figure
- `../docs_src/shared/` - shared template, CSS, and JS for generated docs
- `../docs_src/en/main.html` - English docs content fragment
- `../docs_src/en/page.json` - English docs metadata
- `../docs_src/no/main.html` - Norwegian docs content fragment
- `../docs_src/no/page.json` - Norwegian docs metadata
- `../docs_src/nl/main.html` - Dutch docs content fragment
- `../docs_src/nl/page.json` - Dutch docs metadata
- `../docs_src/pt/main.html` - Portuguese docs content fragment
- `../docs_src/pt/page.json` - Portuguese docs metadata
- `../docs_src/es/main.html` - Spanish docs content fragment
- `../docs_src/es/page.json` - Spanish docs metadata
- `../docs_src/de/main.html` - German docs content fragment
- `../docs_src/de/page.json` - German docs metadata
- `../docs_src/it/main.html` - Italian docs content fragment
- `../docs_src/it/page.json` - Italian docs metadata

Related SPIFFS content (outside this folder):

- `../ui/index.html` - backup/restore web UI (single source, served at `/`)

## Update workflow

1. Edit docs sources:
   - update `source/ESPEED32/docs_src/<lang>/main.html`
   - update shared files in `source/ESPEED32/docs_src/shared/` when layout/CSS/JS should change for all languages
2. Regenerate docs locally when you want preview/build output:
   - terminal: `python3 scripts/build_docs.py`
   - automatic when building/uploading SPIFFS through `scripts/refresh_generated_docs.sh`, `scripts/upload_spiffs.sh`, or `scripts/flash_all.sh`
3. Commit to Git.
4. If firmware and docs changed, run a full flash:
   - VS Code task: `ESPEED32: Flash firmware + SPIFFS`
   - or terminal: `./scripts/flash_all.sh`
5. Upload SPIFFS image only (when firmware is unchanged):
   - VS Code task: `ESPEED32: Upload SPIFFS docs`
   - or terminal: `./scripts/upload_spiffs.sh`
6. Open `http://192.168.4.1/docs` while the controller is in WiFi mode.

## Runtime routes

Firmware serves docs from:

- `/docs` (language-aware default)
- `/docs/en`
- `/docs/no`
- `/docs/nl`
- `/docs/pt`
- `/docs/es`
- `/docs/de`
- `/docs/it`

Routes are implemented in `source/ESPEED32/connectivity_portal.cpp`.

## Optional GitHub Pages hosting

You can host these docs online (for example on GitHub Pages) without duplicating content.

Recommended approach:

1. Keep `source/ESPEED32/data/docs/` as the generated/published output for device and Pages.
2. Keep generator sources in `source/ESPEED32/docs_src/`.
3. In GitHub Pages CI, regenerate docs from `source/ESPEED32/docs_src/` before publishing `source/ESPEED32/data/docs/`.
4. Optionally publish `source/ESPEED32/data/ui/index.html` online too.

Repository workflow:

- `.github/workflows/pages.yml` publishes:
  - regenerates docs via `scripts/refresh_generated_docs.sh`
  - `/docs/` from `source/ESPEED32/data/docs/`
  - `/ui/` from `source/ESPEED32/data/ui/`
  - `/` (landing) from `source/ESPEED32/data/ui/index.html`
- Triggered automatically on push to `main` when docs source/output, docs generator, or hosted UI files change.

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
