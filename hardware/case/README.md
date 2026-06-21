# Two-Node Radar Case Design

Node A and Node B require equivalent wearable enclosures for ESP32, BU01, ST7789, GY-511, MAX30102, and the protected battery power chain.

Mechanical requirements:

- Keep ESP32 and BU01 antenna regions clear of metal and the battery.
- Keep the MAX30102 window aligned with stable skin contact.
- Mount GY-511 flat with its X/Y orientation recorded relative to display top.
- Expose the ST7789 without loading its flex cable or header.
- Provide access to charging, power, reset/boot, and any future interaction button.
- Leave room for regulator and radio decoupling capacitors.

Use the same external orientation for both nodes where possible so axis mapping and radar behavior remain consistent.
