#include "_gpio.h"

void HAL_RCC_GPIO_CLK_ENABLE(GPIO_TypeDef *PORT) {
    if (PORT == GPIOA)
        __HAL_RCC_GPIOA_CLK_ENABLE ();
}
void HAL_RCC_GPIO_CLK_DISABLE(GPIO_TypeDef *PORT) {
    
}