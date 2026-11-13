# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

---

## [1.1.0] - 2026-11-13

### Added
- **Smooth Fan Speed (`smooth_fan_speed` in `number`):** Optional `0..100 %` (`1 %` step) slider that divides `[sfs_min_speed .. sfs_max_speed]` into 3 equal gear zones (`Speed 1`, `Speed 2`, `Speed 3`), sets the active gear's supply/exhaust fan speed to the selected percentage while resetting the other two gears to `sfs_speed_1..3_default` (`30 / 60 / 90 %`), turns ventilation `Off` at `0 %`, and activates `Airing` at `100 %`. Includes compile-time YAML schema validation (`sfs_min_speed < sfs_max_speed` and strictly ascending defaults within bounds).
- **Smooth `0..100 %` `fan` Entity Mode & Extended Presets:** When `smooth_fan_speed` is configured, the `fan` platform automatically upgrades from 3 discrete speeds to a smooth `0..100 %` (`speed_count = 100`) slider alongside `Speed 1`, `Speed 2`, `Speed 3`, `Airing`, `Auto`, and `Party` preset buttons (while preserving the original 3-speed `Auto` / `Party` behavior when `smooth_fan_speed` is omitted).
- **Heat-Recovery Efficiency Sensors (`sensor`):** Added `supply_efficiency` and `exhaust_efficiency` (`%`, `0.1 %` resolution) calculated from `preheater_temperature`, `supply_temperature`, `extract_temperature`, and `exhaust_temperature` (excluding pre-heater heat input; reports `unavailable` when $|\Delta T| < 1.0\ ^\circ\text{C}$).
- **Multi-Packet `'W'` Writes in a Single `F0 44` Window:** When more than 6 registers change at once, a second `'W'` write packet is automatically scheduled 40 ms after the first `'w'` confirmation if still within the first 500 ms of the ~880 ms iNext silence window.
- **Project Documentation:** Added `CHANGELOG.md` and `AGENTS.md`.

### Changed
- **Standard ESPHome Platform Subpackage Layout:** Reorganized `components/awenta_auros/` into the idiomatic ESPHome multi-platform directory structure (`climate/`, `fan/`, `select/`, `switch/`, `number/`, `sensor/`, `binary_sensor/`, `text_sensor/` subpackages pairing Python schemas with their C++ entity headers/sources, while keeping hub & C14 protocol modules `c14_protocol.{h,cpp}`, `awenta_auros.{h,cpp}`, `awenta_auros_parser.cpp`, and `awenta_auros_rwt.cpp` at the component root).

### Fixed
- **Immediate `'w'` Write-Response Slot Confirmation:** Decoded echoed `[reg_id, value]` slots in `'w'` (`0x77`) responses identically to `'r'` (`0x72`) responses so `current_raw` updates and `has_desired` clears immediately (~200 ms after TX) instead of waiting up to ~38 s for the next `E3 44` (`0x39`) broadcast page.
- **AERO 4A2 Fan Speed Telemetry Mapping:** Corrected `supply_fan_speed` and `exhaust_fan_speed` / `extract_fan_speed` byte assignments across `E4 21` (`f[25..26]`) and `E5 21` (`f[14..15]`) so `intake_fan_speed` / `supply_fan_speed` both report the supply fan (`%`) and `extract_fan_speed` / `exhaust_fan_speed` both report the exhaust fan (`%`).

---

## [1.0.0] - 2026-11-12

### Added
- **Supply & Exhaust Fan Speed Setpoints (`number`):** Configuration sliders (`0..100 %`, `1 %` step, `entity_category: config`) to adjust **Speed 1**, **Speed 2**, **Speed 3**, and **Airing** fan percentages for both supply and exhaust fans via the iNext (`F0 44`) write slot:
  - `supply_fan_speed_1` (Reg `141`), `supply_fan_speed_2` (Reg `142`), `supply_fan_speed_3` (Reg `143`), `supply_fan_airing_speed` (Reg `144`)
  - `exhaust_fan_speed_1` (Reg `145`), `exhaust_fan_speed_2` (Reg `146`), `exhaust_fan_speed_3` (Reg `147`), `exhaust_fan_airing_speed` (Reg `148`)
- **Passive Fan Speed Configuration Sensors (`sensor`):** 8 diagnostic sensors (`entity_category: diagnostic`) passively decoded from `E3 44` page `0x39` frames (`f[14..15]` and `f[20..25]`):
  - `config_supply_fan_speed_1..3`, `config_supply_fan_airing_speed`
  - `config_exhaust_fan_speed_1..3`, `config_exhaust_fan_airing_speed`
- **Schedule Mode Binary Sensor (`binary_sensor`):** `schedule_mode` (`entity_category: diagnostic`) decoded from `E4 21` byte `f[5] & 0x10` indicating when the panel operates on time-zone schedules (`Schedule` or `Party`).
- **Default MDI Icons:** Descriptive default Material Design Icons across all `sensor`, `binary_sensor`, `text_sensor`, and `switch` entities (including `mdi:tunnel-outline` for GHE / ground heat exchanger and directional chevron icons for supply vs. extract/exhaust paths).

### Changed
- **Breaking — Sensor Key Standardization & Entity Categories:**
  - Renamed panel configuration/status sensors for consistency:
    - `master_fan_speed` → `active_fan_speed` (`text_sensor`)
    - `season_mode` → `config_season_mode` (`text_sensor`)
    - `ventilation_mode` → `config_airing` (`binary_sensor`)
    - `away` → `config_away` (`binary_sensor`)
    - `fireplace_mode` → `config_fireplace_mode` (`binary_sensor`)
  - Assigned `entity_category: diagnostic` to all `config_*` sensors (`sensor`, `binary_sensor`, `text_sensor`), `thermostat_mode`, and `schedule_mode`.
  - Assigned `entity_category: config` to `preheater_min_temperature` (`number`).

### Fixed
- **Fixed `thermostat_mode` Decoding in `E4 21`:** Switched `thermostat_mode` (`text_sensor`) from `f[5] & 0x50` to `f[27] & 0x03` so `Party` mode (`0x01`) is properly distinguished from `Schedule` (`0x00`) and `Manual` (`0x02`).
- **Refined Unparsed Byte Debug Logging:** Updated `Unparsed ... bytes` debug log statements and `docs/PROTOCOL.md` so newly decoded bytes in `E3 44` (`0x39`, `0x3A`, `0x3B`), `E4 21`, and `E5 21` are no longer logged as unknown.

---

## [0.5.0] - 2026-11-12

### Added
- **Initial Public Release of `awenta_auros` ESPHome External Component:**
  - Passive decoding of Compit C14 (`M.Bus` / RS-485 `9600 8N1`, 30-byte frames) broadcast traffic between **Compit NANO COLOR 2** (Master `41` / `0x29`) and **Compit AERO 4A2** (Slave `99` / `0x63`).
  - Non-intrusive two-way control without replacing or reconfiguring the Master wall panel by transmitting 6-slot `'W'` (`0x57`) register write frames from the **Compit iNext (`112` / `0x70`)** address inside the ~880 ms silence window following `F0 44`.
  - State-diff register synchronization (`desired != current`) with automatic retry.
  - Full ESPHome platform suite:
    - `climate` (modes `Heat`, `Heat/Cool`, `Fan only`, `Cool`; presets `Eco`, `Boost`, `Home`; fan modes `Off`, `Low`, `Medium`, `High`, `Diffuse`, `Auto`)
    - `fan` (3-speed fan with `Auto` and `Party` presets for Apple HomeKit compatibility)
    - `select` (`fan_mode`, `thermostat_mode`, `bypass_mode`)
    - `switch` (`airing`, `fireplace`, `ghe`)
    - `number` (`comfort_temperature`, `eco_winter_temperature`, `eco_cooling_temperature`, `manual_temperature`, `away_temperature`, `preheater_min_temperature` on Reg `161`)
    - `sensor`, `binary_sensor`, and `text_sensor` for temperatures, humidity, fan speeds, setpoints, and operating states
  - Protocol and register documentation in `docs/PROTOCOL.md` and `docs/REGISTERS.md` (including NANO COLOR 2 v6.02 `+1` register shift on `160..201`).

[Unreleased]: https://github.com/d-lebed/esphome-awenta-auros/compare/v1.1.0...HEAD
[1.1.0]: https://github.com/d-lebed/esphome-awenta-auros/compare/v1.0.0...v1.1.0
[1.0.0]: https://github.com/d-lebed/esphome-awenta-auros/compare/v0.5.0...v1.0.0
[0.5.0]: https://github.com/d-lebed/esphome-awenta-auros/releases/tag/v0.5.0
