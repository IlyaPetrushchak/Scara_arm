# SCARA Robot Controller

Open-source controller project for the **PyBot SCARA robotic arm** and compatible SCARA robots, based on an **ESP32 D1 Mini**, a custom PCB and Bluetooth gamepad control.

The repository contains firmware, editable EasyEDA electronics sources, Gerber files, technical documentation and 3D-printing resources used in the project.

## Current firmware

The current `main` branch includes:

- manual JOINT and TOOL control,
- TEACH point storage in ESP32 NVS,
- automatic PTP playback,
- AUTO speed override from 50% to 250%,
- startup OLED logo,
- assisted manual Z-axis HOME procedure after every restart,
- software E-STOP latch requiring controller restart,
- Polish, English and Ukrainian source comments,
- servo 2 startup position of 90° and allowed movement range of 90–180°.

## Startup procedure

ARM1 and ARM2 are positioned manually at `0°` before power-up or reset. After the controller starts, the firmware requires the operator to establish the Z-axis zero position:

1. Wait for the 5-second OLED startup logo.
2. Connect the Bluetooth gamepad and leave all controls neutral until the gamepad becomes armed.
3. Use the right stick Y axis to move Z at the reduced startup homing speed.
4. Set the mechanism physically at the intended `Z = 0` position.
5. Release the stick and wait until Z stops.
6. Press **A+B together** to confirm the current position as `Z = 0`.
7. Release A+B and leave the controller neutral again for normal MANUAL control to arm.

Normal MANUAL/AUTO operation and the MODE/TEACH panel buttons remain locked until Z HOME is confirmed.

## Controls

In MANUAL mode, `START` switches between JOINT and TOOL control.

- JOINT: left stick controls ARM1/ARM2, right stick Y controls Z.
- TOOL: left stick controls TCP X/Y, right stick Y controls Z.
- Right stick X controls tool rotation.
- A increases servo 2 angle, B decreases it; servo 2 is limited to 90–180°.
- PCB `MODE` switches MANUAL/AUTO when all axes are stopped.
- PCB `TEACH`: short press stores a point, hold for at least 1.8 s to clear the stored program.

In AUTO mode, `START` performs start/pause/resume. D-pad UP/DOWN changes speed by ±10%, while D-pad LEFT restores 100%.

## Documentation

- [Documentation index](docs/README.md)
- [Robot operating instructions – Polish](docs/INSTRUKCJA_OBSLUGI_PL.md)
- [AUTO / TEACH reference](docs/AUTO_TEACH.md)
- [Technical documentation – Polish](docs/DOKUMENTACJA_TECHNICZNA_PL.md)
- [Technical documentation – Ukrainian](docs/DOKUMENTACJA_TECHNICZNA_UA.md)

## Repository contents

- `src/` – ESP32 controller firmware.
- `PCB/` – schematic, Gerbers, silkscreen and PCB documentation.
- `PCB/EasyEDA/` – editable EasyEDA schematic and PCB source files.
- `3DPrinting/` – printable mechanical parts.
- `docs/` – operating and technical documentation.

## Build and upload

1. Install Visual Studio Code.
2. Install the PlatformIO extension.
3. Download or clone this repository.
4. Open the project folder in PlatformIO.
5. Connect the ESP32.
6. Build and upload the firmware.

## Credits

### PyBot / jjRobots

The mechanical construction of this robot is based on the **PyBot SCARA Robotic Arm** by **jjRobots**.

Many thanks to jjRobots for publishing the original mechanical design, printable parts and assembly concept that formed the mechanical basis of this project.

- https://www.jjrobots.com/scara-robotic-arm-by-jjrobots/
- https://cults3d.com/en/3d-model/gadget/scara-robotic-arm-open-source-with-control-app

### Alternative gripper

The alternative gripper is based on **Robot Gripper 9g Micro Servo** by **yisparyan**:

- https://www.thingiverse.com/thing:715525

Many thanks to yisparyan for sharing the design with the robotics community.

## License

Original content created for this repository is released under the **MIT License**, unless otherwise marked. This includes the original controller firmware, project documentation and original project files.

Third-party material keeps its original license:

- PyBot-derived mechanical geometry: **CC BY-NC-SA** as published by jjRobots.
- Alternative gripper derived from Thingiverse thing 715525 by yisparyan: **CC BY**.
- External software dependencies retain their own upstream licenses.

Because the PyBot mechanical design includes a **NonCommercial** restriction, those particular third-party 3D assets are not relicensed under MIT and should not be treated as commercially unrestricted open-source content.

See [`LICENSE`](LICENSE), [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md) and [`3DPrinting/README.md`](3DPrinting/README.md) for details.
