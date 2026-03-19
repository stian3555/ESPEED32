# ESPEED32
Cutting edge slot car controller with an ESP32 at its heart.
How to build it — video: https://www.youtube.com/watch?v=JtMKeiguHKI
Hosted Web UI/docs: https://espeed32.com (best in Chrome/Edge or another WebSerial-capable browser)
![ThumbV2](https://github.com/user-attachments/assets/9b7e1479-4882-4ed7-93d0-4a9a81be73fc)

## Magnetic Trigger Sensor

The trigger position is read from a magnetic angle sensor over I2C. Several sensors are supported.

Default build uses `TLE493D`.

Recommended selection method is a compile-time override, for example:

```bash
./scripts/flash_all.sh --compile-only --sensor as5600
```

You can also pass the family directly to `arduino-cli`:

```bash
arduino-cli compile \
  --fqbn esp32:esp32:esp32 \
  --board-options "JTAGAdapter=default,PSRAM=disabled,PartitionScheme=default,CPUFreq=240,FlashMode=qio,FlashFreq=80,FlashSize=4M,UploadSpeed=115200,LoopCore=1,EventsCore=1,DebugLevel=none,EraseFlash=none,ZigbeeMode=default" \
  --build-path build-as5600 \
  --build-property build.extra_flags="-DTRIGGER_SENSOR_FAMILY=TRIGGER_SENSOR_FAMILY_AS5600" \
  source/ESPEED32
```

Legacy manual selection in `source/ESPEED32/HAL.h` still works if you prefer editing the header directly.

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

Supported auto-detect variants:

| Family | Part / suffix example | 7-bit I2C address | Notes |
|---|---|---|---|
| W2B6 | `TLE493D-W2B6 A0` / `TLE493D-W2B6 A0 HTSA1` | `0x35` | Newly supported W2B6 A0 variant |
| W2B6 | `TLE493D-W2B6 A3` | `0x44` | Existing W2B6 support |
| P3B6 | `TLE493D-P3B6 A0` | `0x5D` | Version 3 silicon |
| P3B6 | `TLE493D-P3B6 A1` | `0x13` |  |
| P3B6 | `TLE493D-P3B6 A2` | `0x29` |  |
| P3B6 | `TLE493D-P3B6 A3` | `0x46` |  |

If detection fails, firmware continues and prints a warning on serial output.

> **Silicon revision note:** `TLE493D-P3B6 A0` is version 3 silicon and is available on Digikey/Mouser. The older P2 revision has known issues; some users have managed to get it working, but it is not guaranteed.

## Motor Current Sense Profiles

The firmware now keeps the original motor current path as the default, but allows advanced builders to select a different **current-sense profile** at compile time.

Current profile defines live in `source/ESPEED32/HAL.h`:

- `CURRENT_SENSE_PROFILE_BTN99X0` = original/default hardware behavior
- `CURRENT_SENSE_PROFILE_NONE` = disables current-sense-dependent behavior
- `CURRENT_SENSE_PROFILE_BTS7960` = initial BTS7960 / IBT_2 current-sense profile

Important:

- The default build is still `CURRENT_SENSE_PROFILE_BTN99X0`
- `CURRENT_SENSE_PROFILE_BTS7960` currently changes **current measurement only**
- it is **not** a complete alternate motor-driver implementation
- the BTS7960 values are starting points and may need tuning for the exact module and resistor/filter network used

### Default build

Nothing needs to be changed for the standard controller hardware.

Use the normal flow:

- `./scripts/flash_all.sh`
- or `./scripts/flash_all.sh --compile-only`

### Custom BTS7960 / IBT_2 current-sense build

If you want to try the BTS7960 current-sense profile **without changing the default in the repo**, override it at compile time:

```bash
arduino-cli compile \
  --fqbn esp32:esp32:esp32 \
  --board-options "JTAGAdapter=default,PSRAM=disabled,PartitionScheme=default,CPUFreq=240,FlashMode=qio,FlashFreq=80,FlashSize=4M,UploadSpeed=115200,LoopCore=1,EventsCore=1,DebugLevel=none,EraseFlash=none,ZigbeeMode=default" \
  --build-path build-bts7960 \
  --build-property build.extra_flags="-DCURRENT_SENSE_PROFILE=2" \
  source/ESPEED32
```

Optional tuning overrides for the BTS7960 profile:

```bash
--build-property build.extra_flags="-DCURRENT_SENSE_PROFILE=2 -DBTS7960_CURRENT_SENSE_KILIS=8500 -DBTS7960_CURRENT_SENSE_EFFECTIVE_R_OHMS=1000 -DBTS7960_CURRENT_SENSE_OFFSET_MV=0"
```

What the BTS7960 tuning values mean:

- `BTS7960_CURRENT_SENSE_KILIS`: datasheet current-sense ratio starting point
- `BTS7960_CURRENT_SENSE_EFFECTIVE_R_OHMS`: effective load resistance seen by the `IS` pin at the ADC input
- `BTS7960_CURRENT_SENSE_OFFSET_MV`: optional offset trim if the module shows idle bias

Current status of the BTS7960 profile:

- it compiles successfully
- it uses a first-pass `kILIS` / resistor-based conversion model
- it likely still needs validation and tuning on real IBT_2 / BTS7960 hardware
- if the module does not expose a usable `IS` signal, use `CURRENT_SENSE_PROFILE_NONE` instead

Behavior when current sense is unavailable:

- current readout is shown as `N/A`
- lap detection stays disabled
- self-test skips current-specific checks cleanly

## External Pot Overrides

The firmware also has an initial **external potentiometer override** feature for live control of:

- `BRAKE`
- `SENSI`

Current hardware mapping in firmware:

- `POT1` = `GPIO35`
- `POT2` = `GPIO15`

Current behavior:

- `POT1` and `POT2` are configured from `Settings -> EXT POT`
- each pot can be assigned to `OFF`, `BRAKE`, or `SENSI`
- the pots act as **live global overrides**
- they do **not** overwrite the stored car values in flash
- the UI/race screen shows `POT1` / `POT2` when an override is active
- if both pots are assigned to the same target, the older/conflicting assignment is forced back to `OFF`

Important hardware note:

- `GPIO15` is an `ADC2` pin on ESP32, so it may be unreliable for analog reading while WiFi is active

Please verify the actual board routing against:

- `ESPEED32_V3_0_schematic_PCB.pdf`

before soldering to spare pads or assuming a given pad name matches the expected ESP32 GPIO.

## On-Device Documentation (NO/EN/ES/DE/IT)

User documentation served by the controller lives in:

- `source/ESPEED32/data/docs/en/index.html`
- `source/ESPEED32/data/docs/no/index.html`
- `source/ESPEED32/data/docs/es/index.html`
- `source/ESPEED32/data/docs/de/index.html`
- `source/ESPEED32/data/docs/it/index.html`
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
