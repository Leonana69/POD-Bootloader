# POD-Bootloader

This project is based on the crazyflie2-stm-bootloader. It uses STM32CubeMX to generate the orignal structure and uses HAL library instead of STM32 Standard Peripheral Libraries (SPL), which is deprecated.

## STM32CubeMX Setup

1. System Core -> RCC -> HSE -> Crystal/Ceramic Resonator
2. ![Clock Configuration](/Docs/clock-configuration.png)