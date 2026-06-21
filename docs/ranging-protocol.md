# Two-Node Communication Protocol

The current product uses two independent links between Node A and Node B.

## UWB Distance Link

| Node | Fixed role | EUI base |
|---|---|---|
| Node A | Anchor | `0C:8A:D3:7C:E5:A4` |
| Node B | Tag | `1C:75:C4:F4:E9:D4` |

The tested ranging sequence remains:

```text
BLINK -> RANGING_INIT -> POLL -> RANGE_REPORT
```

Current ranging constants:

- `DW1000.MODE_LONGDATA_RANGE_ACCURACY`
- Antenna delay `16555`
- Application median filter: 5 accepted samples
- Discovery recovery: 5000 ms
- Connected recovery: 15000 ms

The production UI marks UWB stale after 3000 ms without a valid range, but keeps the last Peer dot indefinitely as an inactive outline.

## ESP-NOW Peer Data Link

ESP-NOW operates in Wi-Fi station mode without an AP.

```cpp
struct PeerStatusPacket {
  uint8_t version;
  uint8_t senderNodeId;
  uint16_t sequence;
  uint16_t bpm;
  float peerBearingDeg;
  uint8_t flags;
};
```

Flags identify valid BPM and valid bearing fields.

- Send status every 500 ms.
- Send immediately when a bearing estimate is accepted.
- Use local packet-receive time for freshness.
- Mark Peer BPM unavailable after 3000 ms without a packet.
- On a valid received bearing, display `normalizeAngle(peerBearingDeg + 180)`.

## Independent Failure States

| UWB | ESP-NOW | Display behavior |
|---|---|---|
| Current | Current | Active Peer dot and Peer BPM |
| Stale | Current | Inactive last-known dot and current Peer BPM |
| Current | Stale | Active Peer dot and `PEER BPM:--` |
| Stale | Stale | Inactive last-known dot and `PEER BPM:--` |

## Diagnostic Compatibility

`firmware/tests/uwb_pair_test` keeps the tested environment names `master` and `tag`, plus its AP monitor. In current terminology these map to Node A/Anchor and Node B/Tag. The AP is diagnostic-only and is not part of production firmware.
