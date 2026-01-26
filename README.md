# LED‑Panel‑ESP12F

A compact ESP‑12F–based controller board designed to drive an 8×8 LED panel with an onboard buzzer, stable power regulation, and reliable programming support. This project was completed with the generous support of my sponsor, whose fabrication assistance transformed a fragile prototype into a robust, reproducible hardware platform.

## Sponsor Acknowledgment

This project reached completion thanks to the support of <a href="https://www.pcbway.com/" target="_blank" title="PCBWay">PCBWay</a>, who provided fabrication assistance that transformed the prototype into a clean, reliable PCB. Before fabrication, the prototype relied on a dense web of jumper wires, loose connections, and constant troubleshooting. With my sponsor’s help, the design moved from that unstable setup to a clean, professionally manufactured PCB that is easy to assemble, reliable to operate, and ready for long‑term use.

Their contribution directly improved electrical stability, mechanical reliability, assembly clarity, and long‑term maintainability. The contrast between the prototype and the final board demonstrates the real value of proper PCB fabrication.

## Before and After

### Before: Breadboard-based Prototype  
The initial version worked, but the wiring was messy, fragile, and difficult to debug. It served its purpose for early validation, but it was not suitable for repeated use or documentation.  

<img src="https://github.com/easai/LED-Panel-ESP12F/blob/main/images/IMG_2775.jpeg" width="300" alt="LED-Panel-ESP12F Prototype" />

### After: Assembled PCB  
The final PCB was fabricated by PCBWay, whose manufacturing quality made the design stable, durable, and easy to assemble. The board programs correctly, executes firmware without issues, and drives the LED panel and buzzer exactly as intended.  

<img src="https://github.com/easai/LED-Panel-ESP12F/blob/main/images/IMG_2857.jpeg" width="300" alt="LED-Panel-ESP12F PCB Assembled" />

This comparison highlights the impact of fabrication support and the improvement in overall quality.

## Programming the ESP‑12F Using an FTDI Adapter

The ESP‑12F can be programmed directly through a USB‑to‑serial adapter such as an FTDI module. The board includes a dedicated programming header, making the upload process reliable and repeatable.

To enter programming mode, the ESP‑12F requires specific boot pin states during reset. The PCB handles these automatically through pull‑ups and pull‑downs, so only four connections are needed from the FTDI adapter:

Before uploading, connect the FTDI module to the programming header as shown below.

• FTDI TX → Programming header TX  
• FTDI RX → Programming header RX  
• FTDI GND → Programming header GND  

<img src="https://github.com/easai/LED-Panel-ESP12F/blob/main/images/IMG_2858.jpeg" width="300" alt="LED-Panel-ESP12F PCB Assembled" />

Once connected:

1. Press and hold the BOOT button.  
2. Tap RESET.  
3. Release BOOT.  
4. Upload the firmware from your IDE (Arduino IDE or PlatformIO).  

The board will automatically reboot into normal operation after programming. This workflow ensures consistent flashing without the wiring errors common in breadboard prototypes.


## Hardware Overview

The board integrates an ESP‑12F module, 5 V to 3.3 V regulation, LED panel connector, buzzer with mute control, programming header, clear silkscreen labeling, and a stable power and ground layout. All hardware files are included for reproducibility.

## Repository Structure

hardware/kicad/ contains the KiCad schematic and PCB files  
hardware/fabrication/ contains Gerbers, BOM
hardware/images/ contains schematic, PCB front/back, prototype wiring, and assembled PCB images  
README.md is this document

## Bill of Materials

A complete BOM is available in <a href="https://github.com/easai/LED-Panel-ESP12F/blob/main/hardware/fabrication/LED-Panel-ESP12F-bom.csv" target="_blank" title="LED-Panel-ESP12F BOM">hardware/fabrication/LED-Panel-ESP12F-bom.csv</a>.

## Firmware

The firmware will be rewritten and updated in this repository. The current version demonstrates correct boot behavior, successful program upload, LED panel operation, and buzzer functionality. Future updates will include a cleaner structure and expanded features.

## Credits

PCB fabrication provided by PCBWay.

The LED matrix case used in this project is my own remix, published at <a href="https://www.printables.com/" title="Printables.com" target="_blank">Printables</a>:  
<a href="https://www.printables.com/model/1476658-4-in-1-led-dot-matrix-mount" title="4-in-1 LED Dot Matrix Mount" target="_blank">4-in-1 LED Dot Matrix Mount</a>


## License

MIT License
