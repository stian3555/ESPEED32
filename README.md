# SlotEsp32
Cutting edge slot car controller with an ESP32 at its heart.
How to build it — video: https://www.youtube.com/watch?v=JtMKeiguHKI
![ThumbV2](https://github.com/user-attachments/assets/9b7e1479-4882-4ed7-93d0-4a9a81be73fc)

## Magnetic Trigger Sensor

The trigger position is read from a magnetic angle sensor over I2C. Several sensors are supported.

To select your sensor, open `source/ESPEED32/HAL.h` and uncomment **exactly one** of the `#define` lines in the sensor selection block — leave the rest commented out.

### Supported sensors

| Define | Sensor |
|---|---|
| `TLE493D_MAG` | Infineon TLE493D *(active by default)* |
| `AS5600_MAG` | AMS AS5600 |
| `AS5600L_MAG` | AMS AS5600L (different I2C address variant) |
| `MT6701_MAG` | MagnTek MT6701 |
| `ANALOG_TRIG` | Analog potentiometer (no I2C) |

I2C pins for the sensor: **SDA = GPIO21, SCL = GPIO22**

---

### TLE493D — address variants

The last part of the TLE493D part number indicates the I2C address. Check your exact part number before ordering or building.

| Part number suffix | 7-bit I2C address |
|---|---|
| A0 (e.g. TLE493D-P3B6 **A0**) | 0x5D |
| A1 | 0x13 |
| A3 (tested, W2B6 A3) | 0x44 |

For other variants, check the datasheet for the correct address.

If your variant is not A3, update `ADDRESS` at the top of the `TLE493D_MAG` block in `source/ESPEED32/HAL.cpp`.

> **Silicon revision note:** The TLE493D-P3B6 A0 is version 3 silicon and is available on Digikey/Mouser. The older P2 revision has known issues — some users have managed to get it working, but it is not guaranteed.

## On-Device Documentation (NO/EN)

User documentation served by the controller lives in:

- `source/ESPEED32/data/docs/en/index.html`
- `source/ESPEED32/data/docs/no/index.html`
- `source/ESPEED32/data/ui/index.html` (backup/restore web UI)

Recommended location is exactly this path under `source/ESPEED32/data/docs/` (do not move to repo root),
because ESP32 filesystem tooling expects the `data/` folder next to the sketch.

The firmware serves these files from SPIFFS at `/docs` in WiFi backup mode.
The WiFi backup root page (`/`) is also served from SPIFFS (`/ui/index.html`) with a minimal built-in fallback if the file is missing.

The docs include:

- full controller usage guide
- full UI overview
- full menu tree (main menu and all submenus)
- Norwegian and English versions

### Update workflow

1. Edit docs HTML files in `source/ESPEED32/data/docs/`.
2. Commit to Git.
3. Flash firmware + SPIFFS in one go (recommended after firmware/docs changes):
   - VS Code task: `ESPEED32: Flash firmware + SPIFFS`
   - or terminal: `./scripts/flash_all.sh`
4. Upload filesystem image (SPIFFS) only (when firmware is unchanged):
   - VS Code task: `ESPEED32: Upload SPIFFS docs`
   - or terminal: `./scripts/upload_spiffs.sh`
   - optional: purge SPIFFS region first: `./scripts/upload_spiffs.sh --purge` (asks for confirmation)
5. Open `http://192.168.4.1/docs` while the controller is in WiFi mode.
