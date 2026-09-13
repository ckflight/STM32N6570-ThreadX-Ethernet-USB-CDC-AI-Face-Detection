# STM32N6 AI + USB + Ethernet Project Setup

This project combines **STM32N6 AI (Neural-ART/NPU), USB and Ethernet/NetX Duo** on the STM32N6570-DK.

## 1. Generate the AI Reference Project

Required repositories:

```text
~/stm32ai-modelzoo
~/stm32ai-modelzoo-services
```

Download models and initialize the STM32N6 reference project:

```bash
cd ~/stm32ai-modelzoo
git lfs pull

cd ~/stm32ai-modelzoo-services
git submodule update --init application_code/face_detection/STM32N6
```

Configure:

```text
face_detection/user_config.yaml
```

Then generate/deploy:

```bash
cd ~/stm32ai-modelzoo-services/face_detection
python3.12 stm32ai_main.py
```

Generated/reference project:

```text
/home/ck/stm32ai-modelzoo-services/application_code/face_detection/STM32N6/
```

## 2. Copy AI Files into Our Project

From the generated STM32N6 reference project, copy the required AI integration files into our project.

Main directories:

```text
stedgeai-lib/
Model/
```

These contain the generated **STAI runtime/network integration, Neural-ART configuration, model files and post-processing support**.

Also copy/adapt the required STM32N6 AI initialization, camera pipeline and external-memory configuration from the reference project.

Runtime flow:

```text
Camera → DCMIPP → NN Input → stai_network_run()
       → NN Output → Post-Processing → Application
```

## 3. Add USB

Enable **USB OTG + USBX CDC ACM** in CubeMX.

USB buffers are placed in the dedicated non-cacheable USB RAM region.

```text
USB_RAM → 0x341F8000
```

## 4. Add Ethernet

Enable:

```text
ETH
ThreadX
NetX Duo
```

Add the STM32N6 Ethernet driver/HAL sources and NetX Duo application files.

Ethernet descriptors and NetX memory are placed in the dedicated Ethernet RAM region.

```text
ETH_RAM → 0x341EA000
```

DMA/cache coherency must be handled correctly for Ethernet RX/TX buffers.

## 5. External Memories

The project uses:

```text
External NOR   → AI/model binaries
External PSRAM → Camera / large frame buffers
Internal SRAM  → Application, USB and Ethernet buffers
```

Required BSP, XSPI, MPU/cache and RIF configuration is based on the working Model Zoo STM32N6 reference project.

## Final Project

```text
STM32N6570-DK
│
├── ThreadX
├── USBX CDC
├── NetX Duo / Ethernet
├── Camera / DCMIPP
├── Neural-ART NPU
├── STAI generated model
├── AI post-processing
├── External NOR
└── External PSRAM
```

The Model Zoo project is kept only as the **reference/generation project**. The final application is maintained independently in our own STM32CubeIDE project.
