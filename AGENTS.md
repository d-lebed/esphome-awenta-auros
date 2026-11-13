# AGENTS.md — Contributor & AI Agent Guide

This repository contains the **`awenta_auros`** ESPHome external component for **Awenta Pro Auros** heat-recovery ventilation units (recuperators) equipped with the **Compit AERO 4A2** controller (`Address 99`) and **Compit NANO COLOR 2** touch display panel (`Address 41`, firmware `v6.02`) communicating over the proprietary **Compit C14 (RS-485 / M.Bus, `9600 8N1`)** bus.

---

## 1. Mandatory Rules for Every Change

1. **Always Update `CHANGELOG.md`:**
   - **Every** user-visible, protocol, architectural, bug-fix, or configuration change **must** include a corresponding entry under the `## [Unreleased]` section of [`CHANGELOG.md`](./CHANGELOG.md) (`Added`, `Changed`, `Deprecated`, `Removed`, `Fixed`, or `Security`) in the same commit or branch before finishing the task.
   - When cutting a new Git tag / GitHub Release (`vX.Y.Z`), move the contents of `## [Unreleased]` into `## [X.Y.Z] - YYYY-MM-DD`, update comparison links at the bottom of `CHANGELOG.md`, and leave an empty `## [Unreleased]` section at the top.
2. **Keep Documentation & Example YAML in Sync:**
   - When adding, renaming, or modifying configuration options, entities, icons, or entity categories:
     - Update [`README.md`](./README.md)
     - Update [`example.yaml`](./example.yaml)
   - When discovering or refining C14 frame bytes or register mappings:
     - Update [`docs/PROTOCOL.md`](./docs/PROTOCOL.md) and/or [`docs/REGISTERS.md`](./docs/REGISTERS.md)
     - Remove newly known bytes from `ESP_LOGD(TAG, "Unparsed ... bytes: ...")` in `awenta_auros_parser.cpp` so known fields are never logged as unknown.
3. **Language Conventions:**
   - All repository files, code comments, log messages, commit messages, pull requests, and release notes **must be written in English**.
4. **Commit Conventions:**
   - Follow **Conventional Commits** (`feat: ...`, `fix: ...`, `refactor: ...`, `docs: ...`, `chore: ...`).
   - Develop and test changes on the `dev` branch before rebasing/merging into `main` for tagged releases (`@vX.Y.Z`).

---

## 2. Repository Layout & Architecture

```text
components/awenta_auros/
├── __init__.py                 # Hub schema (uart_id, flow_control_pin, time_id)
├── climate.py                  # Climate platform schema & codegen
├── fan.py                      # Fan platform schema & codegen
├── select.py                   # Select platform (fan_mode, thermostat_mode, bypass_mode)
├── switch.py                   # Switch platform (airing, fireplace, ghe)
├── number.py                   # Number platform (setpoints, fan speeds, smooth_fan_speed)
├── sensor.py                   # Sensor platform (telemetry, efficiency, diagnostic config_*)
├── binary_sensor.py            # Binary sensor platform (demands, bypass, diagnostic config_*)
├── text_sensor.py              # Text sensor platform (active gear, diagnostic modes, clock)
├── c14_protocol.h / .cpp       # C14 addresses, register IDs, 7-bit codec, checksum, enum strings
├── awenta_auros.h / .cpp       # AwentaAurosComponent hub declaration & UART byte-sync loop
├── awenta_auros_parser.cpp     # Passive C14 broadcast decoder (E4 21, E5 21, E3 44) & efficiency
├── awenta_auros_rwt.cpp        # Register state-diff manager, iNext F0 44 TX/RX, smooth_fan_speed
├── awenta_auros_climate.h/.cpp # AwentaAurosClimate implementation
├── awenta_auros_fan.h / .cpp   # AwentaAurosFan implementation (3-speed & 0..100% smooth mode)
└── awenta_auros_entities.h     # Parented Select, Switch, and Number entity subclasses
docs/
├── PROTOCOL.md                 # C14 30-byte frame format, broadcast cycle, iNext F0 44 timing
└── REGISTERS.md                # Verified NANO COLOR 2 (v6.02) + AERO 4A2 register map
```

### Important ESPHome Header Inclusion Rule
ESPHome's build system (`writer.copy_src_tree`) collects **all** `.h` files in `components/awenta_auros/` and `#include`s every one of them into the generated `esphome.h` in **alphabetical order** (`awenta_auros_climate.h`, `awenta_auros_entities.h`, `awenta_auros_fan.h`, `awenta_auros.h`, `c14_protocol.h`).
- Every `.h` file **must** use `#pragma once` and be completely self-contained regardless of inclusion order:
  - Headers that only hold `AwentaAurosComponent *parent_` (`awenta_auros_climate.h`, `awenta_auros_fan.h`) forward-declare `class AwentaAurosComponent;`.
  - Headers that inherit `Parented<AwentaAurosComponent>` (`awenta_auros_entities.h`) must `#include "awenta_auros.h"` at the top.
  - Guard all platform-specific includes, `SUB_*` macros, fields, and classes with `#ifdef USE_CLIMATE`, `#ifdef USE_FAN`, `#ifdef USE_SELECT`, `#ifdef USE_SWITCH`, `#ifdef USE_NUMBER`, `#ifdef USE_SENSOR`, `#ifdef USE_BINARY_SENSOR`, `#ifdef USE_TEXT_SENSOR`, and `#ifdef USE_TIME` so minimal YAML configs compile cleanly when optional platforms are omitted.

---

## 3. Compit C14 Protocol Essentials (Do Not Break!)

1. **Frame Framing & Checksum:**
   - Every C14 frame is strictly **30 bytes** (`f[0]..f[29]`):
     - `f[0] = dst_addr + 128` (`>= 0x80`). **Only `f[0]` ever has bit 7 set.** Any byte `>= 0x80` on UART resynchronizes the RX buffer to start a new frame (`0xFF` idle noise bytes are ignored).
     - `f[1]` = ASCII command (`'!'` `0x21`, `'D'` `0x44`, `'V'` `0x56`, `'R'`/`'r'` `0x52`/`0x72`, `'W'`/`'w'` `0x57`/`0x77`, `'T'`/`'t'` `0x54`/`0x74`).
     - `f[2]` = 7-bit checksum: `(sum of f[0..29] excluding f[2]) & 0x7F`.
     - `f[3]` = 7-bit source address (`41` = Master NANO COLOR 2, `99` = AERO 4A2, `112` = iNext).
     - `f[29]` = `0x23` (`'#'`) terminator.
2. **7-Bit Encoding (`+2000` Offset for Values):**
   - Register IDs (`reg_id`): `H = (reg_id / 128) & 0x7F`, `L = (reg_id % 128) & 0x7F` (no offset).
   - Register values (`raw_val`): `encoded = raw_val + 2000`, `H = (encoded / 128) & 0x7F`, `L = (encoded % 128) & 0x7F`.
   - Unused slots in `'W'` frames **must** have `reg_id = 0` (`0x00, 0x00`) and `d_raw = 2000` (`0x0F, 0x50`).
   - Note on scaling:
     - Integer enums & percentages (`0..100 %`): `raw_val = percent` (e.g., `60 %` → `60`).
     - Room temperature setpoints (Regs `3..7`): tenths of `°C` (`22.5 °C` → `225`).
     - Pre-heater temperatures (Regs `160`, `161`): signed half-degrees `0.5 °C` (`temp_c = raw_val * 0.5f`, `-20` = `-10.0 °C` = `OFF`; in `E3 44` page `0x3A`, `f[10]`/`f[12]` equal `raw_val + 20`).
3. **Non-Intrusive Bus Timing (`F0 44` iNext Window):**
   - **Never transmit outside the `F0 44` silence window.** The NANO COLOR 2 Master (`41`) polls iNext (`112` / `0xF0`) once every ~12.7 s with `F0 44 4E 29 7E...23` and stays silent for **~880 ms**.
   - Schedule `'W'` writes **40 ms** after receiving `F0 44` (`flow_control_pin` HIGH → `1 ms` delay → `write_array` + `flush` → `1 ms` delay → `flow_control_pin` LOW → drain echo bytes).
   - In `'w'` (`0x77`) responses (`~200 ms` after `'W'`), NANO COLOR 2 echoes the 6 `[Reg_H, Reg_L, Val_H, Val_L]` slots in `f[5..28]` (same layout as `'r'`). `handle_rwt_response_` immediately updates `current_raw` and clears `has_desired`; if >6 registers were queued and `< 500 ms` have elapsed in the `F0 44` window, a second `'W'` packet is scheduled in `40 ms`.

---

## 4. Entity Naming & Category Conventions

- **Live Telemetry & Primary Controls (Default Category):**
  - Physical measurements from AERO 4A2 (`99`) or NANO COLOR 2 (`41`) room sensors (`room_temperature`, `room_humidity`, `intake_temperature`, `supply_temperature`, `preheater_temperature`, `exhaust_temperature`, `extract_temperature`, `*_fan_speed`, `supply_efficiency`, `exhaust_efficiency`, `target_temperature`, `active_fan_speed`, `cooling_demand`, `bypass_position`) and primary user controls (`climate`, `fan`, `select`, `switch`, room temperature `number` setpoints, `smooth_fan_speed`).
- **Service / Tuning Controls (`entity_category: config`):**
  - Service-level `number` sliders (`preheater_min_temperature`, `supply_fan_speed_1..3`, `supply_fan_airing_speed`, `exhaust_fan_speed_1..3`, `exhaust_fan_airing_speed`).
- **Read-Back Panel Configuration & Bus Metadata (`entity_category: diagnostic`):**
  - All sensors reading configuration echoed by NANO COLOR 2 must use the `config_` prefix and `ENTITY_CATEGORY_DIAGNOSTIC` (`config_*` in `sensor.py`, `binary_sensor.py`, `text_sensor.py`, plus `thermostat_mode`, `schedule_mode`, `time`, `day_of_week`).
- **Icons:**
  - Every entity schema should define a sensible default `mdi:*` icon (supply/intake airflow uses `*-chevron-down` or `home-import-outline`; extract/exhaust airflow uses `*-chevron-up` or `home-export-outline`; GHE uses `mdi:tunnel-outline`; preheater uses `mdi:heating-coil`).

---

## 5. Validation & Testing Checklist

Before committing changes:
1. Validate Python schema syntax and custom validators (e.g., `cv.Invalid` checks in `number.py`).
2. Compile `example.yaml` (or a full device configuration using ESP-IDF) with ESPHome to verify:
   - Python `CONFIG_SCHEMA` and `to_code` generation succeed.
   - All C++ translation units compile and link without errors under ESP-IDF.
3. Verify `CHANGELOG.md` (`## [Unreleased]`), `README.md`, and `example.yaml` reflect your changes.
