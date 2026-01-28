# LED Panel ESP‑12F

The HTML page is available in this GitHub repository:
[https://easai.github.io/LED-Panel-ESP12F/](https://easai.github.io/LED-Panel-ESP12F/)

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

## Configuration

Copy config.h.sample to config.h and fill in your network details.

config.h contains:
- SECRET_SSID for the WiFi network name
- SECRET_PASS for the WiFi password
- SITE_URL for the target endpoint

config.h is not tracked in the repository. Users must create it before building the firmware.

## Hardware Overview

The board integrates an ESP‑12F module, 5 V to 3.3 V regulation, LED panel connector, buzzer with mute control, programming header, clear silkscreen labeling, and a stable power and ground layout. All hardware files are included for reproducibility.

### Schematic
<a href="https://github.com/easai/LED-Panel-ESP12F/blob/main/hardware/images/LED-Panel-ESP12F-schematic.png" width="300" title="LED-Panel-ESP12F Schematic"><img src="https://github.com/easai/LED-Panel-ESP12F/blob/main/hardware/images/LED-Panel-ESP12F-schematic.png" width="300"/></a>
### Board Layout
<img src="https://github.com/easai/LED-Panel-ESP12F/blob/main/hardware/images/LED-Panel-ESP12F-pcb-front.png" width="300" alt="LED-Panel-ESP12F PCB Front"/>
<img src="https://github.com/easai/LED-Panel-ESP12F/blob/main/hardware/images/LED-Panel-ESP12F-pcb-back.png" width="300" alt="LED-Panel-ESP12F PCB Back"/>
<img src="https://github.com/easai/LED-Panel-ESP12F/blob/main/images/LED-Panel-ESP12F.jpg" width="300" alt="LED-Panel-ESP12F Assembled PCB"/>

## Software

This firmware provides a stable demonstration of the ESP12F board and verifies that the PCB, power routing, and peripherals operate correctly. The program in main.cpp is structured around predictable, nonblocking timing and focuses on hardware validation rather than feature development.

### Boot and Initialization
The program configures all required GPIO pins for the LED panel, buzzer, mute switch, and status LED. It initializes the MAX7219 driver, clears the display, and starts serial output for debugging. This confirms that the ESP12F boots correctly after flashing.

### LED Panel Operation
The MAX7219 driver is initialized, and the panel is updated at fixed intervals using timestamp checks instead of delay. A simple pattern or text is shown to verify panel wiring, power stability, and refresh behavior. The update routine is nonblocking, so the device remains responsive.

### Buzzer and Mute Button
The buzzer is controlled through a digital output pin. The mute button is read continuously and suppresses buzzer output when pressed. This logic runs without blocking the main loop and confirms the correct routing of the switch and buzzer.

### WiFi Startup
WiFi initialization is included to confirm that the module boots into the correct mode and that RF components are functional. Connection status is printed to serial. No network features are implemented in this version.

### Main Loop Structure
The loop uses millisecond timestamps to schedule tasks. LED updates, button reads, and buzzer logic run at defined intervals. No blocking delays are used. This structure ensures reproducible behavior and provides a foundation for future animation or network features.

### Purpose of This Version
This firmware is intended for hardware validation. It confirms correct boot behavior, verifies the LED panel, buzzer, and button, and establishes a stable base for future development.


## Repository Structure

- hardware/kicad/ contains the KiCad schematic and PCB files  
- hardware/fabrication/ contains Gerbers, BOM
- hardware/images/ contains schematic, PCB front/back, prototype wiring, and assembled PCB images  
- README.md is this document

## Bill of Materials

A complete BOM is available in <a href="https://github.com/easai/LED-Panel-ESP12F/blob/main/hardware/fabrication/LED-Panel-ESP12F-bom.csv" target="_blank" title="LED-Panel-ESP12F BOM">hardware/fabrication/LED-Panel-ESP12F-bom.csv</a>.

## Credits

PCB fabrication provided by PCBWay.

The LED matrix case used in this project is my own remix, published at <a href="https://www.printables.com/" title="Printables.com" target="_blank">Printables</a>:  
<a href="https://www.printables.com/model/1476658-4-in-1-led-dot-matrix-mount" title="4-in-1 LED Dot Matrix Mount" target="_blank">4-in-1 LED Dot Matrix Mount</a>


## License

MIT License
