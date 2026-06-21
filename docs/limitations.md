# Radar MVP Limitations

- The Peer angle is a movement-based visual estimate, not a DW1000 Angle-of-Arrival measurement.
- GY-511 has no gyroscope. Horizontal acceleration direction can be noisy and is not reliable dead reckoning.
- Accelerometer direction represents acceleration, not continuous walking velocity.
- The MVP primarily assumes one node moves at a time. Simultaneous movement can produce competing bearing estimates.
- GY-511 mounting orientation must be confirmed through configurable axis mapping.
- Compass readings require real-device calibration and are affected by nearby current, magnets, metal, and enclosure layout.
- UWB antenna orientation, body obstruction, trees, walls, metal, and weak power rails affect range continuity.
- The automatic radar scale changes visual radius; the active `R:5/10/20/40m` label must be read with the dot.
- A retained gray dot after `UWB LOST` is the last estimate, not a live position.
- ESP-NOW Peer BPM freshness is independent from UWB ranging status.
- MAX30102 readings are not medical-grade measurements.
- Battery percentage is an estimate until calibrated against the real discharge curve.
- The current KiCad schematic predates the two-equivalent-node pivot and must not be treated as production wiring.
