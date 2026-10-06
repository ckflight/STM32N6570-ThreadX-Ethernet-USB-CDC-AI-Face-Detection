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

Copy/adapt the required AI files from this reference project:

```text
stedgeai-lib/
Model/
```

These contain the STAI runtime, Neural-ART configuration, generated network files and post-processing support.

## 2. Flash Boot and Debug

Program the external NOR with STM32CubeProgrammer:

```text
0x70000000 → FSBL trusted binary
0x70100000 → AI application trusted binary
0x70380000 → Model/network_data.hex
```

### Flash Boot

This project uses the following FSBL loader:

https://github.com/ckflight/STM32N6570-Loader

The FSBL configures the **system clocks and XSPI NOR memory mapping** before starting the AI application.

Use:

```c
#define DEBUG_MODE 0
```

The AI application uses the clock and XSPI configuration inherited from the FSBL.

Do not reset/reinitialize XSPI2 in the AI application when booting through the FSBL, as this destroys the NOR memory-mapped configuration.

### Direct AI Application Debug

Direct debugging bypasses the FSBL, so the AI application must configure the clocks and XSPI NOR itself.

Use:

```c
#define DEBUG_MODE 1
```

This enables the required:

```text
System clock configuration
XSPI2 / XSPIM reset
NOR initialization
NOR memory-mapped mode
```

### Debug AI Application Through FSBL

To debug the complete **FSBL → AI Application** boot flow, add the AI application's `.elf` to the FSBL debug configuration:

```text
FSBL .elf           → Download: True  | Load symbols: True
AI Application .elf → Download: False | Load symbols: True
```

The debugger starts from the FSBL and keeps source-level symbols available after `BOOT_Application()` jumps to the AI application.

<img width="1991" height="708" alt="Image" src="https://github.com/user-attachments/assets/92dfadd7-1107-462e-b3ec-2940de3dbd35" />

### Attach Debugger After Flash Boot

After programming the FSBL, AI application and `network_data.hex` with STM32CubeProgrammer, set **BOOT0 = LOW** and **BOOT1 = LOW** and power the board normally.

This mode allows the debugger to **attach to the already running application** without resetting or reprogramming it, so the live flash-boot execution can be inspected.

Enable debug access in the FSBL:

```c
__HAL_RCC_BSEC_CLK_ENABLE();
BSEC->AP_UNLOCK = 0xB4;
BSEC->DBGCR     = 0xB451B400;
```

Create a separate **Attach** configuration from the normal AI application debug configuration and change:

```text
Download               → False
Set breakpoint at main → Disabled
```

Power the board normally, then start the Attach configuration to inspect the running flash-boot application.

<img width="1991" height="698" alt="Image" src="https://github.com/user-attachments/assets/38c10fa1-bb4b-46cd-a8af-a58f09d6960a" />

## 3. Application Architecture

The application runs on ThreadX and combines:

```text
Camera / DCMIPP
Neural-ART NPU / STAI
USBX CDC
NetX Duo / Ethernet
```

AI processing flow:

```text
Camera → DCMIPP → NN Input → Neural-ART NPU
       → NN Output → Post-Processing → Application
```

Memory usage:

```text
External NOR   → Application image + AI network data
External PSRAM → Camera / large frame buffers
Internal SRAM  → Application and runtime data

ETH_RAM        → 0x341EA000
USB_RAM        → 0x341F8000
```

USB RAM and Ethernet DMA descriptors are configured as non-cacheable where required.

XSPI, MPU/cache and RIF configuration is based on the working STM32N6 Model Zoo reference project.

## 4. Neural-ART Standalone Flash Boot Issue

When the project was started from the debugger, Neural-ART inference worked correctly.

However, after programming the FSBL, AI application and `network_data.hex` with STM32CubeProgrammer and starting the board normally from flash after a power cycle, the STAI synchronous runtime could stall in:

```text
STAI_RUNNING_WFE
```

The STAI runtime normally waits for NPU events using:

```c
#define LL_ATON_OSAL_WFE() __WFE()
```

For standalone flash boot, the working configuration is:

```c
#define LL_ATON_OSAL_WFE() __NOP()
```

This prevents the Cortex-M55 from entering WFE sleep while waiting for the NPU and keeps the STAI runtime polling until inference continues.

With this change, Neural-ART inference runs correctly after a normal standalone power-on boot without requiring the debugger.

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

## CENK Neural Network Generation

CENK model files were prepared under `~/stm32ai-modelzoo-services/image_classification/` including `cenk_classifier_int8.tflite`, `cenk_classes.txt` and `user_cenk_config.yaml`.

CENK uses a separate Neural-ART configuration with its weights mapped to `0x70400000`.

```bash
cd ~/stm32ai-modelzoo-services/application_code/image_classification/STM32N6/Model
cp user_neuralart_STM32N6570-DK.json cenk_neuralart.json
cp my_mpools/stm32n6-app2_STM32N6570-DK.mpool my_mpools/cenk.mpool
sed -i 's/0x70380000/0x70400000/' my_mpools/cenk.mpool
sed -i 's#stm32n6-app2_STM32N6570-DK.mpool#cenk.mpool#' cenk_neuralart.json
```

Generate CENK STAI / Neural-ART files:

```bash
cd ~/stm32ai-modelzoo-services/image_classification
rm -rf cenk_generated
/opt/ST/STEdgeAI/4.0/Utilities/linux/stedgeai generate --target stm32n6 -m ./cenk_classifier_int8.tflite --name cenk --output ./cenk_generated --workspace ./cenk_generated --st-neural-art default@../application_code/image_classification/STM32N6/Model/cenk_neuralart.json --input-data-type uint8 --inputs-ch-position chlast --output-data-type float32
```

This generates `cenk.c`, `cenk.h`, `cenk_ecblobs.h`, `stai_cenk.c`, `stai_cenk.h` and `cenk_atonbuf.xSPI2.raw` under `cenk_generated/`.

Generate the flashable CENK HEX at `0x70400000`:

```bash
OBJCOPY=$(find /opt/st -type f -name arm-none-eabi-objcopy 2>/dev/null | head -1)
"$OBJCOPY" -I binary -O ihex --change-addresses 0x70400000 cenk_generated/cenk_atonbuf.xSPI2.raw cenk_generated/cenk_data.hex
```

Copy `cenk.c`, `cenk.h`, `cenk_ecblobs.h`, `stai_cenk.c`, `stai_cenk.h` and `cenk_data.hex` to `Model/STM32N6570-DK/`.

Flash layout:

```text
0x70380000 → BlazeFace network_data.hex
0x70400000 → CENK cenk_data.hex
```

## Author

Developed by **Cenk Keskin**.

## License

This project is licensed under the **MIT License**. See the `LICENSE` file for details.