# STM32N6 AI + USB + Ethernet Project Setup

This project combines **STM32N6 Neural-ART/NPU, Camera, USBX CDC and Ethernet/NetX Duo** on the STM32N6570-DK.

<img width="3419" height="3156" alt="Image" src="https://github.com/user-attachments/assets/60a1b6de-f567-4930-a5cc-e23162398121" />
<img width="3855" height="4277" alt="Image" src="https://github.com/user-attachments/assets/a466117d-630e-4bfc-92ac-b6ab001ffb5b" />
<img width="891" height="363" alt="Image" src="https://github.com/user-attachments/assets/ec4c6e51-66e3-4c3a-9b15-2ff8f11c1fbb" />
<img width="903" height="396" alt="Image" src="https://github.com/user-attachments/assets/5a4023f1-26b1-4afc-9f53-fc3d9abb6e7e" />

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

Generate/deploy:

```bash
cd ~/stm32ai-modelzoo-services/face_detection
python3.12 stm32ai_main.py
```

Generated/reference project:

```text
/home/ck/stm32ai-modelzoo-services/application_code/face_detection/STM32N6/
```

## 2. AI Integration

Copy/adapt the required AI files from the generated reference project:

```text
stedgeai-lib/
Model/
```

These contain the STAI runtime, Neural-ART configuration, generated network files and post-processing support.

Runtime flow:

```text
Camera → DCMIPP → NN Input → Neural-ART NPU
       → NN Output → Post-Processing → Application
```

## 3. Flash Boot and Debug Mode

Program the external NOR:

```text
0x70000000 → FSBL trusted binary
0x70100000 → AI application trusted binary
0x70380000 → Model/network_data.hex
```

### Flash Boot

The FSBL configures the **system clocks and XSPI NOR memory mapping** before starting the AI application.

Use:

```c
#define DEBUG_MODE 0
```

The AI application then uses the clock and XSPI configuration inherited from the FSBL and does not reset/reinitialize XSPI2.

### Direct AI Application Debug

Direct debugging bypasses the FSBL, so the AI application must configure the clocks and XSPI NOR itself.

Use:

```c
#define DEBUG_MODE 1
```

This enables the required clock setup, XSPI reset and NOR memory-mapped initialization.

### Attach Debugger After Flash Boot

The FSBL enables debug access with:

```c
__HAL_RCC_BSEC_CLK_ENABLE();
BSEC->AP_UNLOCK = 0xB4;
BSEC->DBGCR     = 0xB451B400;
```

This allows attaching the debugger to the application after a normal FSBL boot.

## 4. USB

Enable:

```text
USB OTG + USBX CDC ACM
```

USB buffers use the dedicated non-cacheable region:

```text
USB_RAM → 0x341F8000
```

## 5. Ethernet

Enable:

```text
ETH
ThreadX
NetX Duo
```

Ethernet descriptors and NetX memory use:

```text
ETH_RAM → 0x341EA000
```

Ethernet DMA/cache coherency must be handled correctly for RX/TX buffers.

## 6. External Memories

```text
External NOR   → Application image + AI network data
External PSRAM → Camera / large frame buffers
Internal SRAM  → Application, USB and Ethernet buffers
```

XSPI, MPU/cache and RIF configuration is based on the working STM32N6 Model Zoo reference project.

## 7. Neural-ART Cold Boot

The STAI synchronous runtime normally waits for NPU events using:

```c
#define LL_ATON_OSAL_WFE() __WFE()
```

Cold flash boot stalled in `STAI_RUNNING_WFE`.

The working configuration is:

```c
#define LL_ATON_OSAL_WFE() __NOP()
```

This prevents the Cortex-M55 from sleeping while waiting for the NPU and keeps the STAI runtime polling until inference continues.

## Final Project

```text
STM32N6570-DK
│
├── FSBL / External NOR Boot
├── ThreadX
├── USBX CDC
├── NetX Duo / Ethernet
├── Camera / DCMIPP
├── Neural-ART NPU
├── STAI Face Detection
├── External NOR
└── External PSRAM
```

The STM32 AI Model Zoo project is used only as the **reference/model-generation project**. The final application is maintained independently in this STM32CubeIDE project.