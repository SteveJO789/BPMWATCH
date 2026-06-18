from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from uuid import uuid5, NAMESPACE_URL


ROOT = Path(__file__).resolve().parents[1]
OUT_DIR = ROOT / "hardware" / "kicad" / "bpmwatch"
SCHEMATIC = OUT_DIR / "bpmwatch.kicad_sch"
PROJECT = OUT_DIR / "bpmwatch.kicad_pro"


def uid(name: str) -> str:
    return str(uuid5(NAMESPACE_URL, f"bpmwatch-kicad:{name}"))


@dataclass(frozen=True)
class Pin:
    number: str
    name: str
    side: str
    y: float
    kind: str = "passive"


@dataclass(frozen=True)
class Module:
    lib_id: str
    ref_prefix: str
    value: str
    pins: tuple[Pin, ...]
    width: float = 34.0
    height: float = 34.0
    datasheet: str = "~"


MODULES = {
    "ESP32": Module(
        "BPMWATCH:ESP32_WROOM_32_MODULE",
        "U",
        "ESP32-WROOM-32 module/dev board",
        (
            Pin("1", "3V3", "left", -14, "power_in"),
            Pin("2", "GND", "left", -10, "power_in"),
            Pin("3", "GPIO18/SCK", "right", -14),
            Pin("4", "GPIO23/MOSI", "right", -10),
            Pin("5", "GPIO19/MISO", "right", -6),
            Pin("6", "GPIO5/CS", "right", -2),
            Pin("7", "GPIO21/SDA", "right", 2, "bidirectional"),
            Pin("8", "GPIO22/SCL", "right", 6),
            Pin("9", "GPIO34/ADC", "right", 10, "input"),
            Pin("10", "GPIO4/SOS", "right", 14, "input"),
            Pin("11", "GPIO16/DC", "right", 18),
            Pin("12", "GPIO17/RST", "right", 22),
            Pin("13", "GPIO27/IRQ", "left", 6, "input"),
            Pin("14", "GPIO14/UWB_RST", "left", 10),
            Pin("15", "EN", "left", 14, "input"),
        ),
        datasheet="https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32_datasheet_en.pdf",
    ),
    "BU01": Module(
        "BPMWATCH:BU01_UWB_BREAKOUT",
        "U",
        "BU01 UWB breakout",
        (
            Pin("1", "3V3", "left", -10, "power_in"),
            Pin("2", "GND", "left", -6, "power_in"),
            Pin("3", "SCK/TX", "right", -10),
            Pin("4", "MOSI/RX", "right", -6, "input"),
            Pin("5", "MISO", "right", -2, "output"),
            Pin("6", "CS", "right", 2, "input"),
            Pin("7", "IRQ", "right", 6, "output"),
            Pin("8", "RST", "right", 10, "input"),
        ),
    ),
    "ST7789": Module(
        "BPMWATCH:ST7789_1IN3_TFT",
        "U",
        "ST7789 1.3 inch TFT",
        (
            Pin("1", "VCC", "left", -12, "power_in"),
            Pin("2", "GND", "left", -8, "power_in"),
            Pin("3", "SCL/SCK", "right", -12),
            Pin("4", "SDA/MOSI", "right", -8),
            Pin("5", "RES", "right", -4, "input"),
            Pin("6", "DC", "right", 0, "input"),
            Pin("7", "CS", "right", 4, "input"),
            Pin("8", "BLK", "right", 8, "input"),
        ),
    ),
    "MAX30102": Module(
        "BPMWATCH:MAX30102_BREAKOUT",
        "U",
        "MAX30102 heart-rate breakout",
        (
            Pin("1", "VIN/3V3", "left", -8, "power_in"),
            Pin("2", "GND", "left", -4, "power_in"),
            Pin("3", "SCL", "right", -6),
            Pin("4", "SDA", "right", -2, "bidirectional"),
            Pin("5", "INT", "right", 2, "output"),
        ),
        datasheet="https://www.analog.com/media/en/technical-documentation/data-sheets/MAX30102.pdf",
    ),
    "LSM303": Module(
        "BPMWATCH:LSM303DLHC_BREAKOUT",
        "U",
        "LSM303DLHC accel/compass breakout",
        (
            Pin("1", "VIN/3V3", "left", -8, "power_in"),
            Pin("2", "GND", "left", -4, "power_in"),
            Pin("3", "SCL", "right", -6),
            Pin("4", "SDA", "right", -2, "bidirectional"),
            Pin("5", "DRDY/INT", "right", 2, "output"),
        ),
        datasheet="https://www.st.com/resource/en/datasheet/lsm303dlhc.pdf",
    ),
    "TP4056": Module(
        "BPMWATCH:TP4056_TYPEC_PROTECTION_MODULE",
        "U",
        "TP4056 Type-C charger with protection",
        (
            Pin("1", "USB_5V", "left", -8, "power_in"),
            Pin("2", "USB_GND", "left", -4, "power_in"),
            Pin("3", "B+", "right", -8, "power_out"),
            Pin("4", "B-", "right", -4, "power_out"),
            Pin("5", "OUT+", "right", 4, "power_out"),
            Pin("6", "OUT-", "right", 8, "power_out"),
        ),
    ),
    "TPS63802": Module(
        "BPMWATCH:TPS63802_BUCK_BOOST_MODULE",
        "U",
        "TPS63802 buck-boost module",
        (
            Pin("1", "VIN", "left", -8, "power_in"),
            Pin("2", "GND", "left", -4, "power_in"),
            Pin("3", "EN", "left", 0, "input"),
            Pin("4", "VOUT_3V3", "right", -8, "power_out"),
            Pin("5", "GND", "right", -4, "power_in"),
        ),
        datasheet="https://www.ti.com/lit/ds/symlink/tps63802.pdf",
    ),
    "BAT": Module(
        "BPMWATCH:602030_LIPO",
        "BT",
        "602030 Li-Po cell",
        (
            Pin("1", "B+", "right", -4, "power_out"),
            Pin("2", "B-", "right", 4, "power_out"),
        ),
        width=20,
        height=16,
    ),
    "SW": Module(
        "BPMWATCH:POWER_SWITCH",
        "SW",
        "Power switch",
        (
            Pin("1", "IN", "left", 0),
            Pin("2", "OUT", "right", 0),
        ),
        width=18,
        height=10,
    ),
    "SOS": Module(
        "BPMWATCH:SOS_BUTTON",
        "SW",
        "SOS button",
        (
            Pin("1", "GPIO", "left", 0),
            Pin("2", "GND", "right", 0),
        ),
        width=18,
        height=10,
    ),
    "CAP": Module(
        "BPMWATCH:POWER_FILTER_CAPACITOR",
        "C",
        "Power-filter capacitor",
        (
            Pin("1", "+", "left", -2),
            Pin("2", "-", "left", 2),
        ),
        width=18,
        height=10,
    ),
    "R": Module(
        "BPMWATCH:BATTERY_DIVIDER_RESISTOR",
        "R",
        "Battery monitor divider",
        (
            Pin("1", "A", "left", 0),
            Pin("2", "B", "right", 0),
        ),
        width=18,
        height=10,
    ),
}


def fmt(n: float) -> str:
    return f"{n:.2f}".rstrip("0").rstrip(".")


def pin_x(module: Module, side: str) -> float:
    if side == "left":
        return -(module.width / 2 + 2.54)
    return module.width / 2 + 2.54


def pin_angle(side: str) -> int:
    return 0 if side == "left" else 180


def pin_end(cx: float, cy: float, module: Module, pin: Pin) -> tuple[float, float]:
    return cx + pin_x(module, pin.side), cy + pin.y


def symbol_def(module: Module) -> str:
    name = module.lib_id.split(":", 1)[1]
    left = -module.width / 2
    right = module.width / 2
    top = -module.height / 2
    bottom = module.height / 2
    pins = []
    for pin in module.pins:
        pins.append(
            f'(pin {pin.kind} line (at {fmt(pin_x(module, pin.side))} {fmt(pin.y)} {pin_angle(pin.side)}) '
            f'(length 2.54) (name "{pin.name}" (effects (font (size 1.27 1.27)))) '
            f'(number "{pin.number}" (effects (font (size 1.27 1.27)))))'
        )
    return f'''
    (symbol "{module.lib_id}" (pin_names (offset 1.016)) (in_bom yes) (on_board yes)
      (property "Reference" "{module.ref_prefix}" (at 0 {fmt(top - 3)} 0) (effects (font (size 1.27 1.27))))
      (property "Value" "{module.value}" (at 0 {fmt(bottom + 3)} 0) (effects (font (size 1.27 1.27))))
      (property "Footprint" "" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (property "Datasheet" "{module.datasheet}" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (symbol "{name}_0_1"
        (rectangle (start {fmt(left)} {fmt(top)}) (end {fmt(right)} {fmt(bottom)}) (stroke (width 0.254) (type default)) (fill (type background))))
      (symbol "{name}_1_1"
        {' '.join(pins)})
    )'''


def placed(module_key: str, ref: str, value: str, x: float, y: float, ds: str | None = None) -> str:
    module = MODULES[module_key]
    properties = [
        f'(property "Reference" "{ref}" (at {fmt(x)} {fmt(y - module.height / 2 - 3)} 0) (effects (font (size 1.27 1.27))))',
        f'(property "Value" "{value}" (at {fmt(x)} {fmt(y + module.height / 2 + 3)} 0) (effects (font (size 1.27 1.27))))',
        f'(property "Footprint" "" (at {fmt(x)} {fmt(y)} 0) (effects (font (size 1.27 1.27)) hide))',
        f'(property "Datasheet" "{ds or module.datasheet}" (at {fmt(x)} {fmt(y)} 0) (effects (font (size 1.27 1.27)) hide))',
    ]
    pins = [f'(pin "{pin.number}" (uuid {uid(ref + pin.number)}))' for pin in module.pins]
    return f'''
  (symbol (lib_id "{module.lib_id}") (at {fmt(x)} {fmt(y)} 0) (unit 1) (in_bom yes) (on_board yes) (dnp no) (uuid {uid(ref)})
    {' '.join(properties)}
    {' '.join(pins)}
  )'''


def label(text: str, x: float, y: float) -> str:
    return f'(label "{text}" (at {fmt(x)} {fmt(y)} 0) (effects (font (size 1.27 1.27))) (uuid {uid("label:" + text + ":" + str(x) + ":" + str(y))}))'


def wire(x1: float, y1: float, x2: float, y2: float, name: str) -> str:
    return f'(wire (pts (xy {fmt(x1)} {fmt(y1)}) (xy {fmt(x2)} {fmt(y2)})) (stroke (width 0) (type default)) (uuid {uid("wire:" + name)}))'


def text(body: str, x: float, y: float, size: float = 1.27) -> str:
    escaped = body.replace('"', '\\"')
    return f'(text "{escaped}" (at {fmt(x)} {fmt(y)} 0) (effects (font (size {fmt(size)} {fmt(size)})) (justify left)) (uuid {uid("text:" + body)}))'


def label_pin(module_key: str, ref: str, x: float, y: float, pin_no: str, net: str, outward: float = 5.0) -> list[str]:
    module = MODULES[module_key]
    pin = next(p for p in module.pins if p.number == pin_no)
    px, py = pin_end(x, y, module, pin)
    lx = px - outward if pin.side == "left" else px + outward
    return [wire(px, py, lx, py, f"{ref}:{pin_no}:{net}"), label(net, lx, py)]


def node_power(prefix: str, x: float, y: float, refs: tuple[str, str, str, str, str]) -> list[str]:
    bat, charger, sw, reg, cap = refs
    items = [
        placed("BAT", bat, f"602030 Li-Po {prefix}", x, y),
        placed("TP4056", charger, f"TP4056 Type-C protected charger {prefix}", x + 42, y),
        placed("SW", sw, f"Power switch {prefix}", x + 82, y + 6),
        placed("TPS63802", reg, f"TPS63802 3.3V buck-boost {prefix}", x + 124, y),
        placed("CAP", cap, f"Bulk/local filter caps {prefix}", x + 164, y + 4),
    ]
    nets = []
    for mod, ref, cx, cy, pins in [
        ("BAT", bat, x, y, {"1": f"VBAT_{prefix}", "2": f"GND_{prefix}"}),
        ("TP4056", charger, x + 42, y, {"1": f"USB5V_{prefix}", "2": f"GND_{prefix}", "3": f"VBAT_{prefix}", "4": f"GND_{prefix}", "5": f"SW_IN_{prefix}", "6": f"GND_{prefix}"}),
        ("SW", sw, x + 82, y + 6, {"1": f"SW_IN_{prefix}", "2": f"VIN_REG_{prefix}"}),
        ("TPS63802", reg, x + 124, y, {"1": f"VIN_REG_{prefix}", "2": f"GND_{prefix}", "3": f"VIN_REG_{prefix}", "4": f"+3V3_{prefix}", "5": f"GND_{prefix}"}),
        ("CAP", cap, x + 164, y + 4, {"1": f"+3V3_{prefix}", "2": f"GND_{prefix}"}),
    ]:
        for pin_no, net in pins.items():
            nets.extend(label_pin(mod, ref, cx, cy, pin_no, net, 4.0))
    return items + nets


def main() -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    lib_symbols = "\n".join(symbol_def(module) for module in MODULES.values())

    parts: list[str] = [
        text("BPMWATCH module-level schematic. One master node plus one slave node repeated for Slave 1 and Slave 2.", 12, 12, 1.7),
        text("BU01 mode/pinout and final ESP32 dev-board pin mapping must be confirmed before PCB layout.", 12, 18),
        text("MASTER NODE", 18, 32, 1.7),
        *node_power("M", 22, 52, ("BT1", "U3", "SW1", "U4", "C1")),
        placed("ESP32", "U1", "ESP32-WROOM-32 master", 70, 110),
        placed("BU01", "U2", "BU01 UWB breakout master", 148, 110),
    ]

    master_nets = {
        "U1": ("ESP32", 70, 110, {"1": "+3V3_M", "2": "GND_M", "3": "UWB_SCK_M", "4": "UWB_MOSI_M", "5": "UWB_MISO_M", "6": "UWB_CS_M", "9": "BAT_ADC_M", "13": "UWB_IRQ_M", "14": "UWB_RST_M"}),
        "U2": ("BU01", 148, 110, {"1": "+3V3_M", "2": "GND_M", "3": "UWB_SCK_M", "4": "UWB_MOSI_M", "5": "UWB_MISO_M", "6": "UWB_CS_M", "7": "UWB_IRQ_M", "8": "UWB_RST_M"}),
    }
    for ref, (mod, x, y, pins) in master_nets.items():
        for pin_no, net in pins.items():
            parts.extend(label_pin(mod, ref, x, y, pin_no, net))

    parts.extend([
        text("SLAVE NODE, BUILD TWO COPIES: Slave 1 and Slave 2", 18, 152, 1.7),
        *node_power("S", 22, 172, ("BT10", "U15", "SW10", "U16", "C10")),
        placed("ESP32", "U10", "ESP32-WROOM-32 slave", 70, 232),
        placed("BU01", "U11", "BU01 UWB breakout slave", 148, 212),
        placed("ST7789", "U12", "ST7789 TFT slave display", 148, 252),
        placed("MAX30102", "U13", "MAX30102 BPM breakout", 230, 212),
        placed("LSM303", "U14", "LSM303DLHC accel/compass breakout", 230, 252),
        placed("SOS", "SW11", "SOS momentary button", 70, 282),
        placed("R", "R10", "Upper battery divider resistor", 128, 282),
        placed("R", "R11", "Lower battery divider resistor", 166, 282),
    ])

    slave_pins = {
        "U10": ("ESP32", 70, 232, {
            "1": "+3V3_S", "2": "GND_S", "3": "SPI_SCK_S", "4": "SPI_MOSI_S", "5": "UWB_MISO_S",
            "6": "UWB_CS_S", "7": "I2C_SDA_S", "8": "I2C_SCL_S", "9": "BAT_ADC_S", "10": "SOS_S",
            "11": "TFT_DC_S", "12": "TFT_RST_S", "13": "UWB_IRQ_S", "14": "UWB_RST_S",
        }),
        "U11": ("BU01", 148, 212, {"1": "+3V3_S", "2": "GND_S", "3": "SPI_SCK_S", "4": "SPI_MOSI_S", "5": "UWB_MISO_S", "6": "UWB_CS_S", "7": "UWB_IRQ_S", "8": "UWB_RST_S"}),
        "U12": ("ST7789", 148, 252, {"1": "+3V3_S", "2": "GND_S", "3": "SPI_SCK_S", "4": "SPI_MOSI_S", "5": "TFT_RST_S", "6": "TFT_DC_S", "7": "TFT_CS_S", "8": "+3V3_S"}),
        "U13": ("MAX30102", 230, 212, {"1": "+3V3_S", "2": "GND_S", "3": "I2C_SCL_S", "4": "I2C_SDA_S", "5": "MAX30102_INT_S"}),
        "U14": ("LSM303", 230, 252, {"1": "+3V3_S", "2": "GND_S", "3": "I2C_SCL_S", "4": "I2C_SDA_S", "5": "LSM303_INT_S"}),
        "SW11": ("SOS", 70, 282, {"1": "SOS_S", "2": "GND_S"}),
        "R10": ("R", 128, 282, {"1": "VBAT_S", "2": "BAT_ADC_S"}),
        "R11": ("R", 166, 282, {"1": "BAT_ADC_S", "2": "GND_S"}),
    }
    for ref, (mod, x, y, pins) in slave_pins.items():
        for pin_no, net in pins.items():
            parts.extend(label_pin(mod, ref, x, y, pin_no, net))

    parts.extend([
        text("Notes:", 18, 316, 1.7),
        text("1. Use 3.3V-compatible logic for BU01, ST7789, MAX30102, and LSM303DLHC.", 18, 324),
        text("2. If BU01 is configured for UART, replace SPI nets with TX/RX nets before firmware bring-up.", 18, 330),
        text("3. Confirm battery divider values from ESP32 ADC range and desired sleep current.", 18, 336),
        text("4. TP4056 modules with protection are not ideal power-path/load-sharing chargers; test charge-while-running behavior on bench.", 18, 342),
    ])

    body = f'''(kicad_sch (version 20250114) (generator "BPMWATCH schematic generator") (uuid {uid("schematic")}) (paper "A3")
  (lib_symbols
{lib_symbols}
  )
{chr(10).join(parts)}
  (sheet_instances (path "/" (page "1")))
)'''
    SCHEMATIC.write_text(body, encoding="utf-8")
    PROJECT.write_text('{"version":1,"generator":"BPMWATCH schematic generator","board":{"design_settings":{}},"schematic":{}}\n', encoding="utf-8")


if __name__ == "__main__":
    main()
