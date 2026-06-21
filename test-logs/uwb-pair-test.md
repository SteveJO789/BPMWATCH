# UWB Pair Test Log

Current product pair: Node A/Anchor (`0C:8A:D3:7C:E5:A4`) and Node B/Tag (`1C:75:C4:F4:E9:D4`). Rows originally recorded as `M-S1` are renamed only for current terminology; numeric evidence is unchanged.

|Date|Pair|Expected Distance|Measured Distance|Quality|Notes|
|-|-|-:|-:|-:|-|
|2026-06-21|Node A-Node B|1.00 m|\~2.48 m raw|\~25-32|Antenna delay 16384, filter off|
|2026-06-21|Node A-Node B|1.00 m|\~0.26 m raw|\~50-70|Antenna delay 16640, filter off|
|2026-06-21|Node A-Node B|1.00 m|\~0.94 m raw|\~50-60|Antenna delay 16555, filter off|
|2026-06-21|Node A-Node B|2.00 m|\~2.32 m raw|\~40-60|Antenna delay 16555, filter off|
|2026-06-21|Node A-Node B|5.00 m|\~4.70 m raw|\~14-51|Antenna delay 16555, filter off; RX -95 to -98 dBm; one reconnect|
|2026-06-21|Node A-Node B|2.00 m|2.03-2.08 m|\~54-79|Antenna delay 16555, filter 5; 170 samples without reconnect|
|2026-06-21|Node A-Node B|5.00 m|4.40-5.55 m; avg 4.999 m|avg 71.6|Antenna delay 16555, filter 5; 471 samples/60s; RX avg -93.44 dBm; no reconnect/recovery|
|2026-06-21|Node A-Node B|10.00 m|9.924 m avg first window; 11.24 m median second window|avg 81.3 first window|Failed stability: one 104.25m outlier, 6 recoveries across 90s; RX avg -95 to -96 dBm|
|2026-06-21|Node A-Node B|10.00 m|9.99-10.24 m; median 10.17 m|avg 79.8|Accepted 30s retest: 138 samples, no outliers, RX avg -96.42 dBm; one peer inactive/recovery|
|2026-06-21|Node A-Node B|20.00 m|20.23-20.29 m; median 20.26 m|avg 78.6|Accuracy passed, continuity limited: 26 samples/30s, RX avg -97.69 dBm, one peer inactive and three recoveries|
