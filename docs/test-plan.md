# Test Plan

## Module Tests

- [ ] ESP32 blink upload succeeds.
- [ ] MAC address prints for all 3 boards.
- [ ] ST7789 displays text and simple map points.
- [ ] MAX30102 reads raw IR and BPM.
- [ ] Battery ADC reads stable values.

## UWB Pair Test

- [ ] Master <-> Slave 1 at 1m, 2m, 5m, 10m.
- [ ] Master <-> Slave 2 at 1m, 2m, 5m, 10m.
- [ ] Slave 1 <-> Slave 2 at 1m, 2m, 5m, 10m.
- [ ] Log errors and quality values.

## UWB Triangle Test

- [ ] Place nodes in a known triangle.
- [ ] Measure all 3 pair distances.
- [ ] Confirm impossible triangles are rejected.
- [ ] Confirm stale pair makes map invalid.

## Map Display Test

- [ ] Master broadcasts `TeamMapPacket`.
- [ ] Slave 1 screen shows Master, S1, S2.
- [ ] Slave 2 screen shows the same map.
- [ ] BPM and battery fields are readable.

## Battery Test

- [ ] Run each node on battery for 30 minutes.
- [ ] Run 3-node system for 1 hour.
- [ ] Check regulator heat and ESP32 resets.

## Field Test

- [ ] Outdoor open space.
- [ ] Trees nearby.
- [ ] Human body blocking signal.
- [ ] Walking movement.
- [ ] Move out of range and return.
