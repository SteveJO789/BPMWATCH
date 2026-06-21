# Radar MVP Test Log

Use one row per controlled movement or failure-state test. For primary MVP tests, move only one node at a time.

| Date | Moving node | Start/end distance | Distance trend | Expected Peer bearing | Node A display | Node B display | UWB state | Peer BPM state | Range band | Notes |
|---|---|---|---|---:|---|---|---|---|---:|---|
| Not tested | Node A | - | decreasing | - | - | - | - | - | - | Production firmware pending |
| Not tested | Node A | - | increasing | - | - | - | - | - | - | Production firmware pending |
| Not tested | Node B | - | decreasing | - | - | - | - | - | - | Production firmware pending |
| Not tested | Node B | - | increasing | - | - | - | - | - | - | Production firmware pending |

## Failure-State Evidence

| Date | Test | Expected | Observed | Pass | Notes |
|---|---|---|---|---|---|
| Not tested | UWB disconnected after valid range | Last dot remains as gray outline with `UWB LOST` | - | - | - |
| Not tested | ESP-NOW stale for 3 seconds | Radar continues; `PEER BPM:--` | - | - | - |
| Not tested | No valid UWB since boot | `WAITING` with no Peer dot | - | - | - |
| Not tested | Distance stable | Previous Peer angle remains | - | - | - |
| Not tested | Auto-range expansion | Band expands above 90% | - | - | - |
| Not tested | Auto-range shrink | Band shrinks after 10 seconds below 35% | - | - | - |
