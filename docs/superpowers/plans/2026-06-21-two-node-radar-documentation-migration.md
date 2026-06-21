# Two-Node Radar Documentation Migration Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make every first-party project document describe the approved two-node Radar MVP while preserving the superseded three-node material as clearly marked legacy history.

**Architecture:** Current documentation uses Node A and Node B as equal Radar Nodes, with fixed DW1000 Anchor/Tag radio roles, UWB distance, GY-511 movement-bearing estimation, ESP-NOW Peer BPM/bearing exchange, and ST7789 radar output. Three-node triangle documents and diagrams move under `docs/legacy/three-node/`; diagnostic UWB pair guides remain in place because their AP monitor and legacy build labels are still useful test tooling.

**Tech Stack:** Markdown, SVG, PlatformIO project paths, PowerShell verification, Git

**Execution Status:** Completed on 2026-06-21; checkboxes below preserve the original execution sequence.

---

### Task 1: Preserve Superseded Three-Node Documents

**Files:**
- Move: `relative_2d_map_architecture.svg` to `docs/legacy/three-node/relative_2d_map_architecture.svg`
- Move: `docs/relative-2d-map.md` to `docs/legacy/three-node/relative-2d-map.md`
- Move: `test-logs/uwb-triangle-test.md` to `docs/legacy/three-node/test-logs/uwb-triangle-test.md`
- Create: `docs/legacy/three-node/README.md`

- [ ] **Step 1: Create the legacy index**

Document that the folder contains the superseded one-Master/two-Slave triangle architecture and that it must not be used for current firmware decisions.

- [ ] **Step 2: Move the historical files without changing their evidence**

Use `apply_patch` moves so Git retains history and no historical content is discarded.

- [ ] **Step 3: Verify legacy isolation**

```powershell
rg -n "TeamMapPacket|Slave 2|triangle" docs/legacy/three-node
```

Expected: matches occur only in the legacy folder and are described as superseded.

### Task 2: Rewrite Current Architecture and Planning Documents

**Files:**
- Modify: `README.md`
- Modify: `BPMWATCH_PROJECT_PLAN_AND_REPO_PROMPT.md`
- Modify: `docs/architecture.md`
- Modify: `docs/ranging-protocol.md`
- Modify: `docs/limitations.md`
- Modify: `docs/simulation/wokwi.md`
- Modify: `hardware/diagrams/README.md`

- [ ] **Step 1: Rewrite the root README**

Describe two equal Radar Nodes, the fixed radio roles, the two data links, the new production path, current diagnostic status, build commands, and links to the approved spec and SVG.

- [ ] **Step 2: Replace the old project plan**

Use the approved MVP phases: documentation migration, common hardware bring-up, production firmware, radar estimation, ESP-NOW Peer data, bench verification, and field/power validation.

- [ ] **Step 3: Rewrite architecture and protocol documents**

Define UWB as distance-only and ESP-NOW as Peer BPM plus reciprocal-bearing transport. Explicitly state that Anchor/Tag does not imply application Master/Slave.

- [ ] **Step 4: Mark the current Wokwi simulation as legacy**

State that `firmware/sim/wokwi` still demonstrates the old triangle UI and is not proof of the Radar MVP until replaced.

### Task 3: Align Hardware Documents

**Files:**
- Modify: `docs/bom.md`
- Modify: `docs/pin-map.md`
- Create: `docs/wiring-node-a.md`
- Create: `docs/wiring-node-b.md`
- Move: `docs/wiring-master.md` to `docs/legacy/three-node/wiring-master.md`
- Move: `docs/wiring-slave.md` to `docs/legacy/three-node/wiring-slave.md`
- Modify: `hardware/case/README.md`
- Modify: `hardware/photos/README.md`
- Modify: `hardware/kicad/bpmwatch/README.md`
- Modify: `hardware/kicad/bpmwatch/FOOTPRINT_RESEARCH.md`

- [ ] **Step 1: Make Node A and Node B hardware-equivalent**

List two each of ESP32, BU01, ST7789, GY-511, MAX30102, battery, charger, regulator, and enclosure hardware.

- [ ] **Step 2: Lock the confirmed bus pins**

Record DW1000 default SPI, ST7789 dedicated HSPI, and shared I2C GPIO21/22 for both nodes.

- [ ] **Step 3: Add role-specific wiring pages**

Node A and Node B use identical wiring; only MAC and fixed DW1000 radio role differ.

- [ ] **Step 4: Mark the current KiCad schematic as pre-pivot**

Do not claim the old Master/repeated-Slave schematic is current. Record that it must be regenerated for two equivalent Radar Nodes before production use.

### Task 4: Align Diagnostic UWB Guides

**Files:**
- Modify: `docs/arduino-ide-uwb-pair-test.md`
- Modify: `docs/arduino-ide-uwb-pair-test-th.md`
- Modify: `docs/uwb-pair-ranging-ap-implementation-th.md`

- [ ] **Step 1: Add a diagnostic-only architecture notice**

Explain that AP/webserver behavior belongs only to `uwb_pair_test`, not production Radar firmware.

- [ ] **Step 2: Map legacy build labels to current node names**

Document `master`/`UWB_IS_MASTER=1` as Node A/Anchor and `tag`/`UWB_IS_MASTER=0` as Node B/Tag. Preserve literal output examples where the existing test still prints legacy labels.

- [ ] **Step 3: Remove obsolete next steps**

Do not direct users to test a third node or triangle ranging after the pair test.

### Task 5: Rewrite Test Plan and Logs

**Files:**
- Modify: `docs/test-plan.md`
- Modify: `test-logs/uwb-pair-test.md`
- Modify: `test-logs/battery-test.md`
- Modify: `test-logs/field-test.md`
- Create: `test-logs/radar-mvp-test.md`

- [ ] **Step 1: Preserve current user-recorded results**

Keep the completed I2C, BPM, charger, GY-511, and UWB measurements. Rename the tested pair from `M-S1` to `Node A-Node B` without altering numeric evidence.

- [ ] **Step 2: Replace triangle/map checks**

Add Node A/B hardware parity, GY-511 axis mapping, motion bearing, reciprocal bearing, UWB lost-dot persistence, ESP-NOW Peer BPM, auto-range, and two-screen radar checks.

- [ ] **Step 3: Add a Radar MVP evidence table**

Capture moving node, distance trend, expected/observed bearing, both screen states, UWB state, Peer BPM state, range band, and notes.

### Task 6: Align Repository Support Documents

**Files:**
- Modify: `firmware/legacy/README.md`
- Modify: `hardware/diagrams/README.md`
- Modify: `hardware/case/README.md`
- Modify: `hardware/photos/README.md`

- [ ] **Step 1: Point each support folder to the current architecture**

Use `radar_2d_map_architecture.svg`, Node A/B naming, and the two-node hardware contract.

- [ ] **Step 2: Explain code migration timing honestly**

State that `firmware/master` and `firmware/slave` are deprecated and scheduled for legacy relocation during firmware implementation; do not claim they have already moved.

### Task 7: Verify and Commit the Documentation Migration

**Files:**
- Verify: all first-party `*.md` and architecture `*.svg` files outside `docs/legacy/`

- [ ] **Step 1: Scan current docs for forbidden architecture terms**

```powershell
rg -n -i "1 master|2 slave|slave 2|three-node|3-node|TeamMapPacket|triangle map" README.md BPMWATCH_PROJECT_PLAN_AND_REPO_PROMPT.md CONTEXT.md docs hardware test-logs firmware/legacy -g "*.md" -g "!docs/legacy/**"
```

Expected: no current-architecture claims; matches are only explicit legacy/diagnostic notices.

- [ ] **Step 2: Check links and referenced paths**

```powershell
@'
import pathlib, re
root = pathlib.Path('.')
missing = []
for path in root.rglob('*.md'):
    if '.pio' in path.parts or 'lib' in path.parts:
        continue
    text = path.read_text(encoding='utf-8')
    for target in re.findall(r'\[[^]]+\]\(([^)#]+)', text):
        if '://' in target or target.startswith('#'):
            continue
        resolved = (path.parent / target).resolve()
        if not resolved.exists():
            missing.append((str(path), target))
print('\n'.join(f'{p}: {t}' for p, t in missing))
raise SystemExit(1 if missing else 0)
'@ | python -
```

Expected: exit 0 with no missing local links.

- [ ] **Step 3: Validate Markdown/SVG hygiene and staged scope**

```powershell
$svgText = Get-Content -Raw -Encoding UTF8 radar_2d_map_architecture.svg
$svg = [xml]$svgText
git diff --check
git status --short
```

Expected: valid SVG XML, no whitespace errors, and user-owned firmware changes remain unstaged.

- [ ] **Step 4: Commit only documentation migration files**

```powershell
git add -- README.md BPMWATCH_PROJECT_PLAN_AND_REPO_PROMPT.md CONTEXT.md docs hardware test-logs firmware/legacy radar_2d_map_architecture.svg
git diff --cached --check
git commit -m "docs: migrate project to two-node radar architecture"
```
