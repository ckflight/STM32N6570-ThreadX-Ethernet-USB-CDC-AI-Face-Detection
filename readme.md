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

Reference project:

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

## 3. USB and Ethernet

Enable:

```text
USB OTG + USBX CDC ACM
ETH + ThreadX + NetX Duo
```

Dedicated memory regions:

```text
ETH_RAM → 0x341EA000
USB_RAM → 0x341F8000
```

USB RAM and Ethernet DMA descriptors are configured as non-cacheable where required.

## 4. External Memories

```text
External NOR   → Application image + AI network data
External PSRAM → Camera / large frame buffers
Internal SRAM  → Executing application + RTOS/USB/Ethernet data
```

XSPI, MPU/cache and RIF configuration is based on the working STM32N6 Model Zoo reference project.

## 5. Flash Boot Layout

Program the three images to external NOR:

```text
0x70000000 → FSBL trusted binary
0x70100000 → AI application trusted binary
0x70380000 → Model/network_data.hex
```

`network_data.hex` is generated with the AI model and is located under the project's `Model` directory.

The FSBL initializes the system/external flash, loads the application and jumps to the application image.

## 6. FSBL / Direct Debug Configuration

The FSBL configures the application clocks with **HCLK = 200 MHz**.

To allow debugger attachment after a normal flash boot, enable BSEC debug access in the FSBL:

```c
__HAL_RCC_BSEC_CLK_ENABLE();
BSEC->AP_UNLOCK = 0xB4;
BSEC->DBGCR     = 0xB451B400;
```

### XSPI reset requirement

When booting through the **FSBL**, do **not** reset XSPI2/XSPIM again in the application's `system_stm32n6xx_fsbl.c`.  
Doing so destroys the external NOR memory-mapped configuration established by the FSBL.

For **direct application debugging without the FSBL**, the XSPI reset and NOR initialization must remain enabled because the FSBL has not configured the external flash.

The project uses `DEBUG_MODE` to select between these two cases:

```text
DEBUG_MODE = 0 → Normal FSBL / flash boot
DEBUG_MODE = 1 → Direct application debug
```

## 7. Neural-ART Cold Boot

The STAI synchronous runtime normally waits for NPU events using:

```c
#define LL_ATON_OSAL_WFE() __WFE()
```

In this project, cold flash boot stalled in `STAI_RUNNING_WFE`, while debugger execution worked correctly.

The working configuration is:

```c
#define LL_ATON_OSAL_WFE() __NOP()
```

This keeps the synchronous STAI runtime polling instead of putting the Cortex-M55 into WFE sleep and allows Neural-ART inference to run correctly after a cold flash boot.

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

The STM32 AI Model Zoo project is used only as the **reference and model-generation project**. The final application is maintained independently in this STM32CubeIDE project.