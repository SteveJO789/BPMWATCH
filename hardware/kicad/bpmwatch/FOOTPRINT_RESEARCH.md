# BPMWATCH Footprint Research

Research date: 2026-06-09

This schematic currently uses module-level symbols. Most module footprints must
be confirmed against the exact purchased breakout board, not only the IC name.

## Recommended Footprint Map

| Schematic symbol value | Qty | Footprint recommendation | Confidence | Notes |
| --- | ---: | --- | --- | --- |
| ESP32-WROOM-32 master/slave | 2 | `RF_Module:ESP32-WROOM-32` or exact variant `RF_Module:ESP32-WROOM-32D` / `RF_Module:ESP32-WROOM-32E` | High | Use this only for the bare ESP32-WROOM castellated module, not an ESP32 dev board. Keep antenna keepout clear. |
| BU01 UWB breakout master/slave | 2 | Custom footprint for Ai-Thinker BU01 module, 23 x 13 x 2.9 mm | Medium | Ai-Thinker lists BU01 size and SPI support. Need pad pattern from datasheet/package drawing before PCB. |
| ST7789 1.3 inch TFT display | 1 | `Connector_PinHeader_2.54mm:PinHeader_1x07_P2.54mm_Vertical` plus board outline/custom mounting holes | Medium | Makerfabs module is 7-pin SPI. Your symbol has 8 pins, so either update symbol/pinout or verify your display board has 8 pins. |
| MAX30102 breakout | 1 | Custom breakout footprint, likely dual-row 2.54 mm header, 18 x 13 mm board | Medium | Common module has 8 pins across two rows. Verify row spacing and board orientation. |
| LSM303DLHC breakout / GY-511 | 1 | `Connector_PinHeader_2.54mm:PinHeader_1x08_P2.54mm_Vertical` plus board outline/custom mounting holes | Medium | GY-511 source lists 8-way straight header, 2.54 mm pitch, 14.5 x 20.5 mm board. |
| TP4056 Type-C protected charger module M/S | 2 | Custom module footprint, around 28 x 17 mm, with pads for IN+/IN-/B+/B-/OUT+/OUT- | Low | Many TP4056 Type-C boards differ. Must match exact board photo and pad spacing. |
| TPS63802 3.3 V buck-boost module M/S | 2 | Custom module footprint if using breakout; bare IC would need TI package footprint and full regulator passives | Low | Current schematic says module, but the exact module vendor is not identified. Do not use bare-IC footprint unless redesigning regulator circuit. |
| 602030 Li-Po M/S | 2 | `Connector_JST:JST_PH_S2B-PH-K_1x02_P2.00mm_Horizontal` or `Connector_JST:JST_PH_B2B-PH-K_1x02_P2.00mm_Vertical` | Medium | Use only if the battery has JST-PH 2.0 plug. Otherwise use solder pads or the exact connector family. Battery body itself is mechanical keepout, not a PCB footprint. |
| Power switch M/S | 2 | `Button_Switch_THT:SW_Slide_1P2T_CK_OS102011MS2Q` or exact switch footprint | Medium | Good starter if using C&K OS102011MS2Q style SPDT slide switch. Confirm pin pitch and actuator direction. |
| SOS momentary button | 1 | `Button_Switch_THT:SW_PUSH_6mm_H4.3mm` or height variant matching purchased tactile switch | Medium | Common 6 x 6 mm through-hole tactile switch. Match height and pin geometry. |
| Battery divider resistors | 2 | `Resistor_SMD:R_0603_1608Metric` or `Resistor_SMD:R_0805_2012Metric` | High | Use 0805 if hand soldering; 0603 is fine for assembly. |
| Bulk/local filter capacitors | 2 | `Capacitor_SMD:C_0805_2012Metric` or `Capacitor_SMD:C_1206_3216Metric` | Medium | Choose by capacitance and voltage rating. Bulk caps such as 10 uF to 22 uF are easier in 0805/1206. |

## Sources Checked

- KiCad official footprint library: ESP32 WROOM footprints are in `RF_Module`.
- Ai-Thinker BU01 product page: BU01 is 23 x 13 x 2.9 mm, 2.8 V to 3.6 V, SPI interface.
- Makerfabs ST7789 1.3 inch TFT page: 240 x 240, SPI 4-wire, 7 pins, 3.3 V only.
- MAX30102 module manual: module is 18 x 13 mm, 8 pins across two rows.
- GY-511 / LSM303DLHC module page: 8-way straight header, 2.54 mm pitch, 14.5 x 20.5 mm.
- KiCad official footprint library: `Connector_PinHeader_2.54mm`, `Connector_JST`, `Button_Switch_THT`, `Capacitor_SMD`, and `Resistor_SMD`.

## Blockers Before Assigning Footprints

1. Annotate the schematic first. The current symbols are still `U?`, `R?`,
   `C?`, `BT?`, and `SW?`, so KiCad netlist output collapses duplicate
   references.
2. Confirm exact purchased breakout boards for BU01, TP4056 Type-C, TPS63802,
   MAX30102, LSM303DLHC, and ST7789.
3. Decide whether the PCB carries bare modules directly or just headers/sockets
   for breakout boards.
4. Fix the ST7789 symbol mismatch if the real display is 7-pin but the schematic
   symbol has 8 pins.
5. Add board outlines and mechanical keepouts for the battery, display, UWB
   antenna area, and ESP32 antenna area.

