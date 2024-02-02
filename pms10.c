#include <stdio.h>
#include "pms10.h"
#include "CH57x_common.h"
#include "CH57x_gpio.h"
#include "CH57x_uart.h"


static uint8_t frame[32];
static struct pms1003data pms_frame;
static uint8_t previous_byte = 0;
static uint8_t current_byte = 0;
static uint8_t byte_index = 0;

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

void pms10_read(void)
{
    if( R8_UART3_RFC )
    {
        previous_byte = current_byte;
        current_byte = R8_UART3_RBR;

        frame[byte_index%32] = current_byte;
        byte_index++;

        if(current_byte == '\x4d' && previous_byte == '\x42')
        {
            frame[0] = '\x42';
            frame[1] = '\x4d';
            byte_index = 2;
            printf("New frame\n\r");
        }
        
        if(byte_index == 31)
        {
            memcpy((uint8_t*)&pms_frame, frame, 32);
            printf("Frame data: ");
            for(int i = 0 ; i <32;i++)
            {
                printf("%02x ", *((uint8_t*)&pms_frame+i));
            }
            printf("\n\r");
            if(pms_frame.frame_len != 28)
            {
                printf("\033[91mPMS10 incorret frame length\033[0m \n\r");
            }
            uint16_t check_sum = 0;
            for(int i = 0; i < 30; i++)
            {
                uint8_t *data = ((uint8_t*)&pms_frame+i);
                check_sum += *data;
            }
            if(check_sum != pms_frame.checksum)
            {
                printf("\033[91mPMS10 checksum incorrect. %04x vs %04x\033[0m\n\r",
                check_sum, pms_frame.checksum);
            }
        }
    }
}