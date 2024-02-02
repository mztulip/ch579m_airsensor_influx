
#include "pms10.h"
#include "CH57x_common.h"
#include "CH57x_gpio.h"
#include "CH57x_uart.h"


void pms10_init(void)
{
    GPIOA_SetBits(GPIO_Pin_5);
    GPIOA_SetBits(GPIO_Pin_2); 
    GPIOA_SetBits(GPIO_Pin_15); 
    GPIOA_ModeCfg(GPIO_Pin_4, GPIO_ModeIN_PU); //PA4 RXD3
    GPIOA_ModeCfg(GPIO_Pin_5, GPIO_ModeOut_PP_5mA); //PA5 TXD3
    GPIOA_ModeCfg(GPIO_Pin_2, GPIO_ModeOut_PP_5mA); //PA2 RESET
    GPIOA_ModeCfg(GPIO_Pin_15, GPIO_ModeOut_PP_5mA); //PA15 SET

    GPIOA_ResetBits(GPIO_Pin_2);
    DelayMs(100); 
    GPIOA_SetBits(GPIO_Pin_2); 

    UART3_DefInit();
    UART3_BaudRateCfg(9600);
}