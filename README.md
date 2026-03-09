# SlotEsp32
Cutting edge slot car controller with an ESP32 at its heart.
How to build it — video: https://www.youtube.com/watch?v=JtMKeiguHKI
Hosted Web UI/docs: https://espeed32.com (best in Chrome/Edge or another WebSerial-capable browser)
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

Current HAL behavior (`source/ESPEED32/HAL.cpp`):

- auto-detects supported TLE493D variants/addresses at boot
- caches detected variant + address in NVS (`sensor_cfg`) for faster next boot
- no manual `ADDRESS` edit is normally required

The part-number suffix still determines the I2C address:

| Part number suffix | 7-bit I2C address |
|---|---|
| A3 (TLE493D-W2B6 **A3**) | 0x44 |
| A0 (e.g. TLE493D-P3B6 **A0**) | 0x5D |
| A1 | 0x13 |
| A2 | 0x29 |
| A3 (TLE493D-P3B6 **A3**) | 0x46 |

If detection fails, firmware continues and prints a warning on serial output.

> **Silicon revision note:** The TLE493D-P3B6 A0 is version 3 silicon and is available on Digikey/Mouser. The older P2 revision has known issues — some users have managed to get it working, but it is not guaranteed.

## On-Device Documentation (NO/EN/ES/DE)

User documentation served by the controller lives in:

- `source/ESPEED32/data/docs/en/index.html`
- `source/ESPEED32/data/docs/no/index.html`
- `source/ESPEED32/data/docs/es/index.html`
- `source/ESPEED32/data/docs/de/index.html`
- `source/ESPEED32/data/ui/index.html` (backup/restore web UI)

Recommended location is exactly this path under `source/ESPEED32/data/docs/` (do not move to repo root),
because ESP32 filesystem tooling expects the `data/` folder next to the sketch.

The firmware serves these files from SPIFFS at `/docs` in WiFi backup mode.
The WiFi backup root page (`/`) is also served from SPIFFS (`/ui/index.html`) with a minimal built-in fallback if the file is missing.

The docs include:

- full controller usage guide
- full UI overview
- full menu tree (main menu and all submenus)
- Norwegian, English, Spanish, and German versions

### Update workflow

1. Flash firmware + SPIFFS in one go (recommended after firmware/docs changes):
   - VS Code task: `ESPEED32: Flash firmware + SPIFFS`
   - or terminal: `./scripts/flash_all.sh`
2. Upload filesystem image (SPIFFS) only (when firmware is unchanged):
   - VS Code task: `ESPEED32: Upload SPIFFS docs`
   - or terminal: `./scripts/upload_spiffs.sh`
   - optional: purge SPIFFS region first: `./scripts/upload_spiffs.sh --purge` (asks for confirmation)
3. Open `http://192.168.4.1/docs` while the controller is in WiFi mode.
