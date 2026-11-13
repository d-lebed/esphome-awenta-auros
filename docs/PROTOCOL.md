# Compit C14 Protocol & iNext Gateway Architecture

This document describes the physical layer, frame format, broadcast cycle, and the **iNext (`112`) R/W/T command slot** used by the **Awenta Pro Auros 305** recuperator (equipped with the **Compit Aero 4A2** controller and **Compit NANO COLOR 2 Wer 6.02** room panel).

---

## 1. Physical Layer & Bus Topology

- **Bus:** RS-485 half-duplex (2-wire `A` / `B`, often labeled `M.Bus` / `C14` on Compit boards)
- **Serial settings:** `9600` baud, `8N1` (8 data bits, no parity, 1 stop bit)
- **Topology:** Multi-drop bus with a single Master:
  - **Compit NANO COLOR 2** (`Address 41` / `0x29`) — **Master** controller (stores user schedules, temperature setpoints, and service parameters in its own RAM/EEPROM and orchestrates all bus communication).
  - **Compit AERO 4A2** (`Address 99` / `0x63`) — **Slave** ventilation actuator board inside the recuperator. It never initiates communication; it only replies when polled by the Master.
  - **Compit iNext** (`Address 112` / `0x70`) — Internet gateway module (emulated by this ESPHome component to write parameters to the Master panel).

---

## 2. 30-Byte C14 Frame Structure

Every C14 packet is strictly **30 bytes** long (`f[0]..f[29]`):

|       Byte | Value             | Description                                                                                                                                                                                                                                                                                                                                                                                  |
| ---------: | ----------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
|     `f[0]` | `0x80 + dst_addr` | **Target address** with bit 7 set (`>= 0x80`). **Only `f[0]` in the entire frame has bit 7 set.** All bytes `f[1]..f[29]` are strictly 7-bit (`< 0x80`).                                                                                                                                                                                                                                     |
|     `f[1]` | ASCII char        | **Command byte:**<br>- `'!'` (`0x21`): Status / configuration broadcast<br>- `'D'` (`0x44`): Master poll / data query<br>- `'V'` (`0x56`): Secondary poll trigger<br>- `'R'` (`0x52`) / `'r'` (`0x72`): Read parameters request / response<br>- `'W'` (`0x57`) / `'w'` (`0x77`): Write parameters request / response<br>- `'T'` (`0x54`) / `'t'` (`0x74`): Read telemetry request / response |
|     `f[2]` | `0x00..0x7F`      | **7-bit Checksum:** `(sum of f[0..29] excluding f[2]) & 0x7F`                                                                                                                                                                                                                                                                                                                                |
|     `f[3]` | `0x00..0x7F`      | **Source address** (plain 7-bit integer, e.g., `41` = `0x29` for Master NANO 1, `99` = `0x63` for AERO, `112` = `0x70` for iNext)                                                                                                                                                                                                                                                            |
|     `f[4]` | `0x00..0x7E`      | Subtype / reserved byte (`0x00` in R/W/T requests, `0x7E` in R/W/T responses, `0x39`/`0x3A`/`0x3B` in `E3 44` pages)                                                                                                                                                                                                                                                                         |
| `f[5..28]` | 24 bytes          | Payload (positional fields for `'!'`/`'D'`, or 6 register slots of 4 bytes each for `'R'`/`'W'`/`'T'`)                                                                                                                                                                                                                                                                                       |
|    `f[29]` | `0x23` (`'#'`)    | **Frame terminator**                                                                                                                                                                                                                                                                                                                                                                         |

### Known C14 Device Addresses

| Address (Dec) | Address (Hex) | Frame Header `f[0]` (`+128`) | Role                                                     |
| ------------: | ------------: | ---------------------------: | -------------------------------------------------------- |
|          `41` |        `0x29` |                       `0xA9` | **NANO 1 (Master room panel)**                           |
|      `42..60` |  `0x2A..0x3C` |                 `0xAA..0xBC` | Secondary NANO Slave panels (`ID 2..20`)                 |
|          `80` |        `0x50` |                       `0xD0` | Floor/heating zone controller                            |
|          `98` |        `0x62` |                       `0xE2` | Auxiliary module                                         |
|          `99` |        `0x63` |                       `0xE3` | **AERO 4A2 ventilation module**                          |
|         `100` |        `0x64` |                       `0xE4` | Primary Broadcast channel (Statuses)                     |
|         `101` |        `0x65` |                       `0xE5` | Secondary Broadcast channel (Config & extract telemetry) |
|         `111` |        `0x6F` |                       `0xEF` | LAN module                                               |
|         `112` |        `0x70` |                       `0xF0` | **iNext / GSM Internet gateway**                         |
|         `113` |        `0x71` |                       `0xF1` | PC service interface                                     |

---

## 3. Value & Register Encoding (7-Bit Pairs)

Because bytes `f[1]..f[29]` must have bit 7 cleared (`< 0x80`), 14-bit integers are split into High (`H`) and Low (`L`) 7-bit bytes:

```cpp
uint16_t raw = ((H & 0x7F) * 128) + (L & 0x7F);
```

- **Register indices (`reg_id`):** encoded directly without offset:
  - `H = (reg_id / 128) & 0x7F`
  - `L = (reg_id % 128) & 0x7F`
- **Register values & temperatures:** encoded with a **`+2000` offset** (to support negative numbers without setting bit 7):
  - `encoded = raw_value + 2000`
  - `H = (encoded / 128) & 0x7F`
  - `L = (encoded % 128) & 0x7F`
  - For temperatures (in `0.1 °C`), `raw_value = round(temp_c * 10)` (e.g. `22.0 °C` → `220 + 2000 = 2220` → `H = 0x11, L = 0x2C`).
  - For integer enums/percentages (`0, 1, 2...`), `raw_value = value` (e.g. `0` → `2000` → `H = 0x0F, L = 0x50`; `5` → `2005` → `H = 0x0F, L = 0x55`).

---

## 4. Passive Broadcast Frames in Each Master Cycle (~12.7s)

During normal operation, the NANO COLOR 2 Master (`41`) continuously runs a ~12.7-second polling cycle:

1. **`E4 21` (`From: 41 -> To: 100`, command `'!'`) — Master Panel Status:**
   - `f[5]`: Mode flags (bit `0x40` = AERO link OK, bit `0x10` = schedule time-zones active in `Schedule`/`Party`, bit `0x04` = Away active).
   - `f[7]`: Day of week (`0` = Monday .. `6` = Sunday).
   - `f[8]`, `f[9]`: Current hour and minute.
   - `f[12..13]`: Current room temperature (`decode_value`).
   - `f[14..15]`: Active target room temperature (`decode_value`).
   - `f[23]`: Room relative humidity (`%`).
   - `f[24]`: Cooling demand flag (`(f[24] & 0x32) == 0x00`).
   - `f[25..26]`: 3-phase rotating date (`0x01` = Year - 2000, `0x02` = Month, `0x03` = Day).
   - `f[27]`: Bitfield:
     - Bits `0..1` (`0x03`): Thermostat mode (`0x00` = Schedule, `0x01` = Party, `0x02` = Manual).
     - Bits `3..4` (`0x18`): Season (`0x00` = Winter, `0x08` = Summer, `0x10` = Cooling).
     - Bit `5` (`0x20`): Airing active.
   - `f[28]`: Active fan gear & Holiday flag:
     - Bits `1..2` (`(f[28] >> 1) & 0x03`): Active fan gear (`0` = Stop, `1` = Gear 1, `2` = Gear 2, `3` = Gear 3).
     - Bit `0` (`0x01`): Normal/Away validity bit (`1` in Normal and Away modes, `0` when **Holiday / Vacation (`Urlop`)** mode is active, e.g. `0x04` instead of `0x05` on Gear 2; controlled via Register `89`).
     - Bit `6` (`0x40`): Normal operating state (`1` in Normal mode, `0` when Away or Holiday program is active).

2. **`E3 44` (`From: 41 -> To: 99`, command `'D'`) — Master Poll & Settings Push to AERO 4A2:**
   - Alternating 3-page configuration (`f[4] = 0x39`, `0x3A`, `0x3B`) containing fan speed percentages for Gears 1/2/3/Airing, heater PID parameters, and sensor thresholds.
   - On page `f[4] = 0x39`:
     - `f[14]`: Exhaust fan Airing speed (`%`, Register `148`).
     - `f[15]`: Supply fan Airing speed (`%`, Register `144`).
     - `f[20..22]`: Exhaust fan Speed 1 / Speed 2 / Speed 3 (`%`, Registers `145..147`).
     - `f[23..25]`: Supply fan Speed 1 / Speed 2 / Speed 3 (`%`, Registers `141..143`).
     - `f[27]`: Bitfield combining Season (`0x00` = Winter, `0x08` = Summer, `0x10` = Cooling), Bypass command (`0..2`), and Airing overlay (`+0x20`).
   - On page `f[4] = 0x3A`:
     - `f[6..9]`: Recirculation `AO3` Speed 1 / Speed 2 / Speed 3 / Airing (`%`, Registers `179..182`).
     - `f[10]`: Pre-heater lock temperature (`temp_c = (f[10] - 20) * 0.5 °C`, `0x00` = `-10.0 °C` = `OFF`; Register `160`).
     - `f[12]`: Minimum temperature before heat exchanger (`temp_c = (f[12] - 20) * 0.5 °C`, `0x00` = `-10.0 °C` = `OFF`; Register `161`).
   - `f[28]`: Bit `6` (`0x40`) indicates **Fireplace Mode active** (`0x41` when ON, `0x01` when OFF; controlled via Register `76`).

3. **`E4 21` (`From: 99 -> To: 100`, command `'!'`) — AERO 4A2 Primary Telemetry (sent ~300ms after `E3 44`):**
   - `f[10..11]`: Intake (outdoor) temperature.
   - `f[12..13]`: Supply air temperature.
   - `f[14..15]`: Pre-heater temperature.
   - `f[18..19]`: Supply target temperature.
   - `f[22]`: Target fan speed (`%`).
   - `f[25]`: Intake fan speed (`%`).
   - `f[26]`: Exhaust fan speed (`%`).
   - `f[28]`: Bypass damper position (bit `0x20` = Open).

4. **`E3 56` (`From: 41 -> To: 99`, command `'V'`) → `E5 21` (`From: 99 -> To: 101`, command `'!'`) — AERO 4A2 Secondary Telemetry:**
   - `f[14]`: Supply fan speed (`%`).
   - `f[15]`: Extract fan speed (`%`).
   - `f[16..17]`: Exhaust air temperature.
   - `f[18..19]`: Extract (indoor return) air temperature.

5. **`E5 21` (`From: 41 -> To: 101`, command `'!'`) — Master Panel Configuration Broadcast:**
   - `f[6..7]`: Current intake temperature.
   - `f[8..9]`: Comfort temperature setpoint (Register `3`).
   - `f[10..11]`: Eco Winter temperature setpoint (Register `4`).
   - `f[12..13]`: Eco Cooling temperature setpoint (Register `5`).
   - `f[14..15]`: Manual temperature setpoint (Register `6`).
   - `f[16..17]`: Away temperature setpoint (Register `7`).
   - `f[25]`:
     - Bits `0..1` (`0x03`): Bypass mode (Register `79`: `0` = Off, `1` = Auto, `2` = On).
     - Bit `2` (`0x04`): GHE / Pre-heater mode (Register `78`: `0` = Off, `4` = Auto).
   - `f[27]`: Season enum (`0x00` = Winter, `0x0A` = Summer, `0x14` = Cooling).
   - `f[28]`: Combined decimal UI mode `therm * 10 + fan_mode`:
     - `fan_mode = f[28] % 10` (Register `80`: `0` = Off, `1` = Speed 1, `2` = Speed 2, `3` = Speed 3, `4` = Party, `5` = Schedule).
     - `therm = (f[28] / 10) % 10` (Register `88`: `0` = Schedule, `1` = Party, `2` = Manual).

---

## 5. How Control Works Without Replacing the Master Panel (iNext Slot `F0 44`)

Unlike approaches that disconnect the display panel or put it into Slave mode, this component coexists with the **NANO COLOR 2** panel remaining in **Master** mode by acting as the **Compit iNext (`112` / `0x70`)** module:

1. **The `F0 44` Silence Window:**
   - Once per cycle (~every 12.7s), right after broadcasting `E5 21`, the Master panel (`41`) polls the iNext address (`112`, `0xF0`):
     ```text
     F0 44 4E 29 7E 7E 7E 7E 7E 7E 7E 7E 7E 7E 7E 7E 7E 7E 7E 7E 7E 7E 7E 7E 7E 7E 7E 7E 7E 23
     ```
   - After sending `F0 44`, the Master panel remains silent for **~880 ms** waiting for a request from iNext (`112`).

2. **Sending R/W/T Commands (`112 -> 41`):**
   - If any register's desired state differs from its current state on the bus (`desired != current`), the ESP32 waits **40 ms** after receiving `F0 44` and transmits a `'W'` (`0x57`) frame addressed to Master `41` (`f[0] = 0xA9`) from iNext `112` (`f[3] = 0x70`).
   - Up to **6 registers** can be written in a single 30-byte `'W'` frame (`f[5..28]`, 4 bytes per slot: `[Reg_H, Reg_L, Val_H, Val_L]`). Unused slots have `Reg = 0` (`0x00, 0x00`) and `Val = 2000` (`0x0F, 0x50`).
   - ~200 ms later (well within the 880 ms window), the Master panel replies with a `'w'` (`0x77`) frame to `112` (`f[0] = 0xF0`) confirming the updated registers, and broadcasts the new state in the next cycle.
