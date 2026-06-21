# Legacy Firmware

This directory is reserved for superseded production firmware and reference sketches.

The current production target is the approved common `firmware/radar_node` project with Node A/Anchor and Node B/Tag build environments.

The existing root-level `firmware/master` and `firmware/slave` projects still contain pre-pivot three-node scaffolding. They are deprecated now and will move under a named three-node legacy folder during firmware implementation, when include paths and build references can be migrated safely.

Do not copy `TeamMapPacket`, triangle-solver, or application Master/Slave behavior into the two-node Radar firmware.

`firmware/tests/uwb_pair_test` is not legacy. It remains the active calibrated two-node UWB diagnostic even though its tested build environments and Serial strings retain the literal names `master` and `tag`.
