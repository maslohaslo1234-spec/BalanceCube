# BalanceCube

A reaction wheel cube that balances on one edge or on one corner using three brushless motors, an ESP32 microcontroller, and an MPU6050 IMU.

Built as part of the Hack Club Fallen program.

---

## Project Overview

<img width="716" height="784" alt="Zrzut ekranu 2026-11-08 210207" src="https://github.com/user-attachments/assets/6e7700b1-cf0f-4893-9fa1-70a5cd27ceec" />

### Features
- **3-Axis Reaction Wheel Balancing:** Powered by three Nidec 24H brushless motors.
- **Custom PCB:** Designed in KiCad for the ESP32 devkit, motor control signals, power distribution from a 3S LiPo, and MPU6050.
- **Lightweight Frame:** 150x150x150mm modular frame with 45° chamfered edges, internal corner brackets, and dedicated component mounts.
- **Dynamic Balancing and Nut Slots:** Reaction wheels has 16 M4 perimeter ballast bolts and captive M3 nut slots for clamping on the shaft.
- **Center of Mass:** 3S LiPo battery mounted opposite to the 3 motors to balance the center.

---

## Hardware Specifications

| Component | Description |
| :--- | :--- |
| **Microcontroller** | ESP32 DevKit (Dual-core, hardware PWM, FreeRTOS) |
| **Sensor** | MPU-6050 6-DOF IMU (Accelerometer + Gyroscope) |
| **Actuators** | 3x Nidec 24H Brushless DC Motors with internal drivers |
| **Power Supply** | 3S LiPo Battery (11.1V - 12.6V) connected to XT30 connector |
| **Voltage Regulator**| L7805 Linear Regulator (12V to 5V for ESP32 VIN) |
| **frame** | 3D Printed with PETG/PLA |
| **Fasteners** | M4x16mm ballast bolts, M3x8mm frame screws |

---

## Mechanical Design

The mechanical parts are modeled in Onshape.
- **Public Onshape Project Link:** [Onshape BalanceCube Document](https://cad.onshape.com/documents/2c01e583674b5bc36f37b82b/w/a163e07f377434656225f53e/e/8aebbe53c625277084c7079e?renderMode=0&uiState=6af1ee5f03c2db71e7140d83)

### Key CAD Models:
- `motor-wall` — Modular wall with cutout for reaction wheel and diagonal mounting bridge.

<img width="986" height="705" alt="image" src="https://github.com/user-attachments/assets/d7fc7438-6bc2-4d26-9f4b-95e7c4915269" />

- `PCB-wall` — Passive wall with a truss structure and integrated PCB standoffs.

<img width="975" height="686" alt="image" src="https://github.com/user-attachments/assets/752e19de-039a-45cd-b56b-8965afda7380" />

- `battery-wall` — Passive wall with support shelves for the 3s LiPo that will be secured with velcro.

<img width="872" height="643" alt="image" src="https://github.com/user-attachments/assets/7404c27c-8ae9-4deb-833d-7b89a3ce8ab4" />

- `corner-bracket` — The frame is assembled using 8 internal corner brackets.

<img width="563" height="453" alt="image" src="https://github.com/user-attachments/assets/591bb39a-b70c-479b-89aa-03ef593c4fed" />

- `Flywheel` — 116mm diameter reaction wheel with 16 ballast holes for M4x16 bolts. To maximize reaction torque without overloading the motors, mass is placed as far from the axis as possible.

<img width="925" height="661" alt="image" src="https://github.com/user-attachments/assets/7a97830f-6378-454f-9c32-a1316fff00cd" />

---

## Electronics

Custom 2-layer PCB designed in KiCad:

<img width="737" height="645" alt="image" src="https://github.com/user-attachments/assets/f7735c41-cf3c-43ca-a07e-bb1750d4e2d3" />

### Key PCB Details:
- The bottom layer is poured as a GND plane.
- Main power is fed from a 3S LiPo pack (11.1V – 12.6V) through an XT30 connector (J1).
- Power enters a 470 µF bulk electrolytic capacitor (C1) which smooths out voltage spikes.
- A dedicated L7805 linear regulator (U1) steps 12V down to 5V to power the ESP32, and it's decoupled by a 100 nF capacitor (C2) on the output.
- The ESP32 plugs into female headers soldered to the board.
- Three JST connectors (J3, J4, J5) break out PWM, Direction, Brake, Encoder, and Power lines from the Nidec 24H motors.
- The MPU6050 module connects with I2C (SDA, SCL) and it's mounted on female headers and standoffs.
- An extension header breaks out 5V, 3.3V, GND and spare GPIOs (GPIO27, GPIO34, GPIO19, GPIO23, GPIO36) for future expansion.

---

## Inspirations & Credits

- [Rem-RC Self-Balancing-Cube](https://github.com/remrc/Self-Balancing-Cube)
- [ETH Zurich Cubli](https://idsc.ethz.ch/research-dandrea/research-projects/archive/cubli.html)
- [The Tinkering Techie](https://www.youtube.com/@TheTinkeringTechie)

---

## License
This project is open-source and licensed under the [MIT License](LICENSE).
