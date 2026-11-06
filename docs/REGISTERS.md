# Compit NANO COLOR 2 (v6.02) + AERO 4A ver.2 Register Map

## C14 R/W/T Addressing Notes
- **Target Address:** `41` (`0x29`, frame header byte `f[0] = 41 + 128 = 0xA9`) — Main NANO 1 Master panel.
- **Sender Address:** `112` (`0x70`) — iNext gateway (or `113` / `0x71` — PC).
- **Transmission Window:** `~40 ms` after receiving the iNext poll frame `F0 44 4E 29 7E...23` (`From: 41 -> To: 112`, ~880 ms silence window on the bus).
- **Register Index in C14:** Uses the **base register index** without the `+1000` or `+2000` offsets (the `1000+` and `2000+` offsets are only used by the Modbus RTU interface).
  - If a register does not exist, NANO COLOR 2 returns `9999` (`0x5D 0x5F`).
- **Slot Encoding in `f[5..28]` (6 slots of 4 bytes each):**
  - Bytes `0..1` of slot — `reg_id`:
    - `H = (reg_id / 128) & 0x7F`
    - `L = (reg_id % 128) & 0x7F`
  - Bytes `2..3` of slot — `raw_val` with `+2000` offset:
    - For integer modes/percentages (`0, 1, 2..100`): `d_raw = val + 2000` (e.g. `0` -> `2000` = `0x0F 0x50`, `5` -> `2005` = `0x0F 0x55`).
    - For temperatures in `0.1 °C`: `d_raw = round(temp_c * 10) + 2000` (e.g. `22.0 °C` -> `220 + 2000 = 2220` = `0x11 0x2C`).

---

## 1. Panel Operating Mode Registers (Commands `'R'` / `'W'`)

| C14 Reg | Modbus RAM (Addr) | iNext Code (`Code=223`) | Description | Allowed Values |
|---:|---:|---|---|---|
| **64** | 1065 (`1064`) | `__trybpracyinstalacji` | **Season Mode** | `0` = Winter<br>`1` = Summer<br>`2` = Cooling |
| **76** | 1077 (`1076`) | — | **Fireplace Mode** | `0` = OFF<br>`1` = ON |
| **77** | 1078 (`1077`) | `__aerowentylacjaon&off` | Ventilation ON/OFF | `0` = OFF<br>`1` = ON |
| **78** | 1079 (`1078`) | `__aerostangwc` | **GHE / Pre-heater** | `0` = OFF<br>`1` = AUTO (ON) |
| **79** | 1080 (`1079`) | `__aerokonfbypass` | **Bypass Mode** | `0` = OFF<br>`1` = AUTO<br>`2` = ON |
| **80** | 1081 (`1080`) | `FAN_MODE` | **Ventilation Mode / Fan Speed** | `0` = Off (Stop)<br>`1` = Speed 1<br>`2` = Speed 2<br>`3` = Speed 3<br>`4` = Party / Holiday<br>`5` = Schedule |
| **81** | 1082 (`1081`) | `__aerowietrzenie` | **Airing / Boost** | `0` = OFF<br>`1` = ON |
| **82** | 1083 (`1082`) | `__aerobiegwtrybiecomfort` | Comfort zone ventilation gear | `0..3` (Gear 0..3) |
| **83** | 1084 (`1083`) | `__aerobiegwtryieautoeco` | Eco zone ventilation gear | `0..3` (Gear 0..3) |
| **84** | 1085 (`1084`) | `__aerobiegwtrybieurlop` | Away mode ventilation gear | `0..3` (Gear 0..3) |
| **85** | 1086 (`1085`) | `__aerolicznikfiltra` | Filter operating days counter | `0..300` days |
| **88** | 1089 (`1088`) | `PRESET_MODE` | **Thermostat Operating Mode** | `0` = Schedule<br>`1` = Party / Holiday<br>`2` = Manual |
| **89** | 1090 (`1089`) | `__urlop` | Holiday | `0` = OFF<br>`1` = ON |
| **98** | 1099 (`1098`) | `__trybpozadomemznano1` | Away Mode | `0` = OFF<br>`1` = ON |

---

## 2. Temperature Setpoints & Basic NANO COLOR 2 Settings (Commands `'R'` / `'W'`)

Temperatures are stored in tenths of a degree (`0.1 °C`), e.g. `220` = `22.0 °C`.

| C14 Reg | Modbus (`Addr`) | iNext Code | Min | Max | Default | Unit | Description |
|---:|---:|---|---:|---:|---:|---|---|
| **1** | 1002 (`1001`) | `__numernano` | 1 | 20 | 1 | — | NANO panel ID on C14 bus |
| **2** | 1003 (`1002`) | — | 0 | 2 | 2 | — | C14 network role (`0`=Slave, `1`=Master Mini, `2`=Master) |
| **3** | 1004 (`1003`) | `__tempzadkomf` | 0 | 400 | 240 | `0.1 °C` | **Comfort Temperature Setpoint** |
| **4** | 1005 (`1004`) | `__tempzadekozima` | 0 | 400 | 180 | `0.1 °C` | **Eco Winter Temperature Setpoint** |
| **5** | 1006 (`1005`) | `__tempzadekochlodzenie` | 0 | 400 | 280 | `0.1 °C` | **Eco Cooling Temperature Setpoint** |
| **6** | 1007 (`1006`) | `__tempzadpracareczna` | 0 | 400 | 200 | `0.1 °C` | **Manual Temperature Setpoint** |
| **7** | 1008 (`1007`) | `__tempzadpozadomem` | 0 | 400 | 150 | `0.1 °C` | **Away Temperature Setpoint** |
| **8** | 1009 (`1008`) | — | -100 | 100 | 0 | `0.1 °C` | Room temperature sensor calibration offset |
| **9** | 1010 (`1009`) | — | 1 | 100 | 5 | `0.1 °C` | Thermostat hysteresis |
| **10** | 1011 (`1010`) | — | 0 | 3 | 0 | — | Winter/Summer switching method |
| **11** | 1012 (`1011`) | `__onoffwentnapierwszekr` | 0 | 1 | 0 | — | Simplified screen mode |
| **13** | 1014 (`1013`) | `__wygaszaczekranu` | 0 | 1 | 1 | — | Screensaver enable |
| **14** | 1015 (`1014`) | `__zezwolenienaedycjenapisow` | 0 | 1 | 1 | — | Allow label editing |
| **15** | 1016 (`1015`) | — | 0 | 2 | 0 | — | Main screen info |
| **16** | 1017 (`1016`) | — | 0 | 3 | 1 | — | Thermostat function |
| **17** | 1018 (`1017`) | `__adr_modbus` | 1 | 254 | 44 | — | Modbus slave address |
| **18** | 1019 (`1018`) | — | 0 | 1 | 0 | — | Factory reset |
| **63** | 1064 (`1063`) | `__beepprzyciskow` | 0 | 1 | 1 | — | Button beep sound |
| **128** | 1129 (`1128`) | `__tempzimastart` | 0 | 400 | 150 | `0.1 °C` | Auto-season: Winter Start |
| **129** | 1130 (`1129`) | `__tempzimastop` | 0 | 400 | 170 | `0.1 °C` | Auto-season: Winter Stop |
| **130** | 1131 (`1130`) | `__tempchlodzstart` | 0 | 400 | 220 | `0.1 °C` | Auto-season: Cooling Start |
| **131** | 1132 (`1131`) | `__tempchlodzstop` | 0 | 400 | 250 | `0.1 °C` | Auto-season: Cooling Stop |

---

## 3. AERO 4A ver.2 Service Parameters in NANO COLOR 2 (Commands `'R'` / `'W'`)

Stored in NANO COLOR 2 memory and cyclically pushed to the AERO 4A2 module via `E3 44` frames (subtypes `0x39`, `0x3A`, `0x3B`).

| C14 Reg | Modbus (`Addr`) | iNext Code | Min | Max | Default | Unit | Description |
|---:|---:|---|---:|---:|---:|---|---|
| **140** | 1141 (`1140`) | `__a4prpozadomem` | 0 | 3 | 0 | — | Away program (`0`=continuous, `1`=30m/30m, `2`=20m/40m, `3`=20m/100m) |
| **141** | 1142 (`1141`) | `__a4wentylacjanawobrbieg1` | 0 | 100 | 30 | `%` | **Supply fan: Speed 1** |
| **142** | 1143 (`1142`) | `__a4wentylacjanawobrbieg2` | 0 | 100 | 60 | `%` | **Supply fan: Speed 2** |
| **143** | 1144 (`1143`) | `__a4wentylacjanawobrbieg3` | 0 | 100 | 90 | `%` | **Supply fan: Speed 3** |
| **144** | 1145 (`1144`) | `__a4wentylacjanawobrwietrz` | 0 | 100 | 100 | `%` | **Supply fan: Airing** |
| **145** | 1146 (`1145`) | `__a4wentylacjawywobrbieg1` | 0 | 100 | 30 | `%` | **Exhaust fan: Speed 1** |
| **146** | 1147 (`1146`) | `__a4wentylacjawywobrbieg2` | 0 | 100 | 60 | `%` | **Exhaust fan: Speed 2** |
| **147** | 1148 (`1147`) | `__a4wentylacjawywobrbieg3` | 0 | 100 | 90 | `%` | **Exhaust fan: Speed 3** |
| **148** | 1149 (`1148`) | `__a4wentylacjawywobrwietrz` | 0 | 100 | 100 | `%` | **Exhaust fan: Airing** |
| **149** | 1150 (`1149`) | `__a4tempnawmin` | 0 | 50 | 16 | `°C` | Minimum supply air temperature |
| **150** | 1151 (`1150`) | `__a4tempnawmaks` | 0 | 50 | 35 | `°C` | Maximum supply air temperature |
| **151** | 1152 (`1151`) | `__a4sterowanienagrzwt` | 0 | 1 | 0 | — | Secondary heater control (`0`=OFF, `1`=ON) |
| **152** | 1153 (`1152`) | `__a4czaswietrzenia` | 0 | 100 | 0 | `min` | Airing duration (`0` = toggle ON/OFF mode) |
| **153** | 1154 (`1153`) | `__a4pracazkominkiem` | 0 | 1 | 0 | — | Enable fireplace feature in service menu (`0`=OFF, `1`=ON) |
| **154** | 1155 (`1154`) | `__a4korobrdlakomin` | 0 | 100 | 10 | `%` | Fan speed offset for fireplace mode |
| **155** | 1156 (`1155`) | `__a4korbiegodsensorow` | 0 | 1 | 1 | — | Automatic speed boost from CO2/RH sensors |
| **156** | 1157 (`1156`) | `__a4stabwydsyscsf` | 0 | 1 | 0 | — | CSF flow stabilization system |
| **157** | 1158 (`1157`) | — | 0 | 2 | 2 | — | Supply temp regulation mode (`0`=Troom, `1`=Tsupply, `2`=Texhaust) |
| **158** | 1159 (`1158`) | — | 0 | 100 | 20 | — | Supply setpoint correction gain (Winter) |
| **159** | 1160 (`1159`) | — | 0 | 100 | 20 | — | Supply setpoint correction gain (Summer/Cooling) |
| **160** | 1161 (`1160`) | — | -20 | 20 | -20 | `°C` | Minimum pre-recuperator temperature |
| **161** | 1162 (`1161`) | — | 10 | 100 | 20 | — | Pre-recuperator regulation band |
| **162** | 1163 (`1162`) | — | 0 | 100 | 50 | — | Secondary heater proportional band |
| **163** | 1164 (`1163`) | — | 0 | 100 | 4 | — | Secondary heater `Ki` gain |
| **164** | 1165 (`1164`) | — | 0 | 100 | 20 | `s` | Fan rundown time |
| **165** | 1166 (`1165`) | — | 0 | 365 | 0 | `days` | Filter replacement interval |
| **166** | 1167 (`1166`) | — | -20 | 100 | 5 | `°C` | Defrost threshold temperature |
| **167** | 1168 (`1167`) | — | 1 | 9 | 3 | `×0.2` | Defrost regulation band |
| **168** | 1169 (`1168`) | — | 0 | 3 | 0 | — | Defrost method (`0`=stop supply fan, `1`=pre-heater, `2`=pre-heater+50%, `3`=bypass) |
| **169** | 1170 (`1169`) | — | 0 | 1 | 1 | — | Use intake temperature as outdoor temperature |
| **170** | 1171 (`1170`) | — | 0 | 3 | 0 | — | Relay R3 function |
| **171** | 1172 (`1171`) | — | 0 | 1 | 1 | — | Digital input DI2 function |
| **172** | 1173 (`1172`) | — | -20 | 40 | 8 | `°C` | GHE Winter threshold |

---

## 4. Humidity / CO2 / VOC Thresholds (Commands `'R'` / `'W'`)

| C14 Reg | Modbus (`Addr`) | iNext Code | Min | Max | Unit | Description |
|---:|---:|---|---:|---:|---|---|
| **210** | 1211 (`1210`) | — | 0 | 2000 | `ppm` | CO2 airing start threshold |
| **211** | 1212 (`1211`) | — | 0 | 2000 | `ppm` | CO2 airing stop threshold |
| **212** | 1213 (`1212`) | — | 0 | 1000 | `ppm` | VOC airing start threshold |
| **213** | 1214 (`1213`) | — | 0 | 1000 | `ppm` | VOC airing stop threshold |
| **214** | 1215 (`1214`) | `__startOsuszania` | 0 | 100 | `%` | Dehumidification start threshold (NANO built-in sensor) |
| **215** | 1216 (`1215`) | `__stopOsuszania` | 0 | 100 | `%` | Dehumidification stop threshold (NANO built-in sensor) |
| **216** | 1217 (`1216`) | `__start_nawil_czuj_nano` | 0 | 100 | `%` | Humidification start threshold (NANO built-in sensor) |
| **217** | 1218 (`1217`) | `__stop_nawil_czuj_nano` | 0 | 100 | `%` | Humidification stop threshold (NANO built-in sensor) |

---

## 5. Read-Only Telemetry Registers (Command `'T'`, Indices `16..54`)

In Modbus RTU these correspond to `2000 + index`. Most of these values are already broadcast passively in `E4 21` and `E5 21` frames.

| `'T'` Index | Modbus (`Addr`) | iNext Code | Unit | Description |
|---:|---:|---|---|---|
| **16** | 2017 (`2016`) | `__rd_hig` | `0.1 %` | Room relative humidity (e.g. `462` = `46.2 %`) |
| **21** | 2022 (`2021`) | `__rt_t_nawiew` | `0.1 °C` | Intake / supply air temperature |
| **22** | 2023 (`2022`) | `__tczerpni` | `0.1 °C` | Extract air temperature |
| **23** | 2024 (`2023`) | `__rt_t_wywiewu` | `0.1 °C` | Exhaust air temperature |
| **24** | 2025 (`2024`) | `__rt_t_wyrzutni` | `0.1 °C` | Auxiliary temperature |
| **25..45** | 2026..2046 | — | — | Pressure switch, defrost, heaters, filter status, fan speeds `%`, gear, bypass, GHE |
| **46, 47** | 2047, 2048 | — | `0.1 °C` | EX4 expansion module temperatures T1, T2 |
| **48, 49, 54** | 2049, 2050, 2055 | `__rt_a4_t5..t7` | `0.1 °C` | AERO 4 temperatures T5, T6, T7 |
| **50** | 2051 (`2050`) | `__rt_akt_moc_ahu` | `%` | Current AHU power |
