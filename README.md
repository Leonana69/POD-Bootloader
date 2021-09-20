# POD-Bootloader

This project is based on the crazyflie2-stm-bootloader. It uses STM32CubeMX to generate the orignal structure and uses HAL library instead of STM32 Standard Peripheral Libraries (SPL), which is deprecated.

## STM32CubeMX Setup

1. **Pinout & Configuration** -> System Core -> RCC -> HSE -> Crystal/Ceramic Resonator

2. **Clock Configuration**:
<p align="center">
    <img src=Docs/clock-configuration.png width="700">
</p>

3. **Pinout & Configuration** -> Connectivity:
    - USART3 (debug)
    - USART6 (communication with NRF51822)
        - NVIC Settings: Enable USART6 global interrupt

4. **Pinout & Configuration** -> System Core -> GPIO -> PD2 (LED blue) -> PC0 (LED green) -> PC1 (LED red).
    - GPIO output level: Low
    - GPIO mode: Output Push Pull
    - GPIO Pull-up/Pull-down: No
    - Maxium output speed: Medium
    - User Label: /

5. **Project Manager**
    - Project -> Toolchain/IDE (Makefile)
    - Code Generator: check Generate perihperal initialization as a pair of '.c/.h' files per peripheral