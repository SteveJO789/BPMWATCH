# Compass, Heading UI, and SOS Test Plan

This phase keeps UWB as distance-only. Compass heading is the local wearer/device heading, not a measured bearing to the peer. The radar dot remains a movement/demo angle and is transformed into a north-oriented UI angle with local heading.

## Pin Assumptions

| Function | ESP32 pin | Notes |
| --- | --- | --- |
| GY-511 / LSM303DLHC SDA | GPIO21 | Shared I2C with MAX30102 |
| GY-511 / LSM303DLHC SCL | GPIO22 | Shared I2C with MAX30102 |
| GY-511 accel I2C | 0x19 or 0x18 | SA0 variant fallback |
| GY-511 magnetometer I2C | 0x1E | Required for heading |
| SOS button | GPIO32 | `INPUT_PULLUP`, active LOW |

## Build Tests

- Native policy tests: `C:\Users\pipat\.platformio\penv\Scripts\platformio.exe test -e native`
- Compass enabled: `C:\Users\pipat\.platformio\penv\Scripts\platformio.exe run -e node_a_anchor_espnow_range`
- Compass disabled: `C:\Users\pipat\.platformio\penv\Scripts\platformio.exe run -e node_a_anchor_espnow_range_no_compass`
- SOS enabled: `C:\Users\pipat\.platformio\penv\Scripts\platformio.exe run -e node_b_tag_espnow_display`
- SOS disabled: `C:\Users\pipat\.platformio\penv\Scripts\platformio.exe run -e node_b_tag_espnow_display_no_sos`

## Compass Test

- Start each node and confirm `COMPASS=OK`.
- Confirm the I2C scan includes `1E` and either `19` or `18`.
- Rotate the device and confirm `HDG` changes through `0..359`.
- Confirm `MAG_RAW=x,y,z`, `CS#`, and `CAGE` update while UWB ranging remains active.
- Optional calibration capture: build with `-D BPMWATCH_COMPASS_CAL_LOG=true`, rotate the node slowly, and record `COMPASS_CAL MAG_MIN=... MAG_MAX=...`.
- Confirm `LR_OK=1`, `LR_FAIL=0`, and `IRQMODE=IRQ` are still present in normal logs.

## ESP-NOW Heading Test

- Start both nodes with ESP-NOW enabled.
- Confirm each node logs `ESPNOW=OK`.
- Rotate Node A and confirm Node B logs/displays `REMOTE_HDG`.
- Rotate Node B and confirm Node A logs `REMOTE_HDG`.

## SOS Test

- Hold the SOS button on Node A for about 1 second.
- Confirm Node A logs `SOS=1`, `SOS_SEQ` increments, and `BTN_EVENT=LONG`.
- Confirm Node B shows the red peer SOS banner and logs `REMOTE_SOS=1`.
- Hold the SOS button again for about 1 second.
- Confirm Node A logs `SOS=0` with a new sequence and Node B clears remote SOS after receiving the cancel packet.
- Power off Node A while SOS is active and confirm Node B clears remote SOS after the 10 second timeout.

## Integrated Test

- Confirm UWB distance still updates at the expected range.
- Confirm ESPNOW counters increment without high `TX_FAIL`.
- Confirm MAX30102 still reports BPM when a finger is present.
- Confirm compass/SOS logs do not spam faster than the normal diagnostics interval.
- Confirm no UI text claims true bearing or peer direction from compass alone.
