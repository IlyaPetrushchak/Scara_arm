# Third-party notices and credits

This repository contains original work from the SCARA Robot Controller project as well as material based on third-party open designs. The root `LICENSE` (MIT) applies only to original project content unless a file or directory is explicitly identified below as third-party material.

## PyBot / jjRobots mechanical design

The mechanical construction of this project is based on the **PyBot SCARA Robotic Arm** created by **jjRobots**.

We would like to thank jjRobots for publishing the original mechanical design, printable parts and assembly concept that made this project possible.

Sources:
- https://www.jjrobots.com/scara-robotic-arm-by-jjrobots/
- https://cults3d.com/en/3d-model/gadget/scara-robotic-arm-open-source-with-control-app

The published PyBot 3D design is distributed under **Creative Commons Attribution-NonCommercial-ShareAlike (CC BY-NC-SA)**. The source page retrieved for this project does not identify the Creative Commons version number, so this notice intentionally does not assume one.

Any PyBot-derived geometry included in this repository remains subject to the original PyBot license and is **not relicensed under MIT**.

## Alternative gripper – yisparyan / Thingiverse 715525

The alternative gripper used in this project is based on **Robot Gripper 9g Micro Servo** by **yisparyan**:

- https://www.thingiverse.com/thing:715525

Thank you to yisparyan for publishing the gripper design and making it available for reuse.

The original model is published under **Creative Commons Attribution (CC BY)**. The source information retrieved for this project does not identify the Creative Commons version number, so this notice intentionally does not assume one.

The file `3DPrinting/Alternative gripper.3mf` is therefore subject to the attribution requirements of the original CC BY license and is **not relicensed under MIT**.

## External software libraries

Libraries and frameworks referenced by `platformio.ini`, including Arduino/ESP32, Bluepad32, FastAccelStepper, ESP32Servo, Adafruit GFX and Adafruit SSD1306, remain subject to their own upstream licenses. They are not relicensed by this repository.

## License scope summary

- Original firmware, PCB-related project files, documentation and other original project material: **MIT License**, unless otherwise marked.
- PyBot-derived mechanical geometry: **CC BY-NC-SA**, as published by jjRobots.
- Alternative gripper derived from Thingiverse thing 715525 by yisparyan: **CC BY**, as published by the original author.
- External dependencies: their respective upstream licenses.

If an upstream license notice conflicts with this summary, the upstream license notice takes precedence for that third-party material.
