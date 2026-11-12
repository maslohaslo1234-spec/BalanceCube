# BalanceCube

<img width="716" height="784" alt="Zrzut ekranu 2026-11-08 210207" src="https://github.com/user-attachments/assets/6e7700b1-cf0f-4893-9fa1-70a5cd27ceec" />

---

## What is this?

It's a 3D printed cube that can (hopefully) balance on one corner or one edge. It uses flywheels, brushless motors, an ESP32, and an IMU to balance the cube.

### Features:
- **Balancing** - using three brushless motors and flywheels.
- **Custom PCB** - Designed in KiCad.
- **Lightweight Frame** - It can almost fly (:
- **Firmware** - It works (optional).

---

## How it works?

It works by spinning the reaction wheels in the opposite way that the cube is tilting. The reaction wheels are spun by brushless motors, and the motors are controlled by the ESP32. The ESP32 gets data from the MPU6050 IMU and processes this data using my code, and then it sends control signals to the motors. This way the cube always stays upright (hopefully).

---

## Why am I making this?

I stumbled upon the Fallen program and started thinking if I would be able to join. I remembered a recent YT video from [The Tinkering Techie channel](https://www.youtube.com/@TheTinkeringTechie) about a balancing cube and thought that it was really cool and that I can make something similar in Fallen.

---

## Mechanical Design

The mechanical parts are modeled in Onshape.

Public Onshape Project Link: [Onshape BalanceCube Document](https://cad.onshape.com/documents/2c01e583674b5bc36f37b82b/w/a163e07f377434656225f53e/e/8aebbe53c625277084c7079e?renderMode=0&uiState=6af1ee5f03c2db71e7140d83)

In the repo it can be found here: [CAD](cad/)

<img width="986" height="705" alt="image" src="https://github.com/user-attachments/assets/d7fc7438-6bc2-4d26-9f4b-95e7c4915269" />
<img width="975" height="686" alt="image" src="https://github.com/user-attachments/assets/752e19de-039a-45cd-b56b-8965afda7380" />
<img width="872" height="643" alt="image" src="https://github.com/user-attachments/assets/7404c27c-8ae9-4deb-833d-7b89a3ce8ab4" />
<img width="563" height="453" alt="image" src="https://github.com/user-attachments/assets/591bb39a-b70c-479b-89aa-03ef593c4fed" />
<img width="925" height="661" alt="image" src="https://github.com/user-attachments/assets/7a97830f-6378-454f-9c32-a1316fff00cd" />

---

## Electronics

The PCB is designed in KiCad.

In the repo it can be found here: [PCB](PCB/)

<img width="1186" height="850" alt="Zrzut ekranu 2026-11-02 201625" src="https://github.com/user-attachments/assets/518d71de-f4b3-4782-a056-321e70569cb5" />

<img width="1082" height="795" alt="image" src="https://github.com/user-attachments/assets/5534d9f3-f17f-4da1-b288-16ead53e563a" />

<img width="1063" height="836" alt="image" src="https://github.com/user-attachments/assets/24d3be63-2686-4d14-ab0d-7af9e95ff19c" />

---

## Bill of Materials (BOM)

All parts, links, and prices can be found in [bom.csv](bom.csv).

---

## Inspirations & Credits

- [Rem-RC Self-Balancing-Cube](https://github.com/remrc/Self-Balancing-Cube)
- [ETH Zurich Cubli](https://idsc.ethz.ch/research-dandrea/research-projects/archive/cubli.html)
- [The Tinkering Techie](https://www.youtube.com/@TheTinkeringTechie)

---

## License

This project is open-source and licensed under the [MIT License](LICENSE).
