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

static bool check_checksum(void)
{
    static struct pms1003data *pms_frame_pointer = (struct pms1003data *)frame;
    if(pms_frame_pointer->frame_len != 28)
    {
        printf("\033[91mPMS10 incorret frame length\033[0m \n\r");
    }

    uint16_t check_sum = 0;
    for(int i = 0; i < 30; i++)
    {
        check_sum += frame[i];
    }
    if(check_sum != pms_frame_pointer->checksum)
    {
        printf("\033[91mPMS10 checksum incorrect. %04x vs %04x\033[0m\n\r",
        check_sum, pms_frame.checksum);
        return false;
    }
    return true;
}

static void print_frame(void)
{
    printf("Frame data: ");
    for(int i = 0 ; i <32;i++)
    {
        printf("%02x ", frame[i]);
    }
    printf("\n\r");
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
        }
        
        if(byte_index == 32)
        {   
            // print_frame();
            if(check_checksum() == true)
            {
                memcpy((uint8_t*)&pms_frame, frame, 32);
            }
        }
    }
}

void pms10_print_data(void)
{
    printf("PM1.0 concentration: %d ug/m3\n\r", pms_frame.pm10_standard);
    printf("PM2.5 concentration: %d ug/m3\n\r", pms_frame.pm25_standard);
    printf("PM10 concentration: %d ug/m3\n\r", pms_frame.pm100_standard);
    printf("Number of particles > 0.3um in 0.1L: %d\n\r", pms_frame.particles_03um);
    printf("Number of particles > 0.5um in 0.1L: %d\n\r", pms_frame.particles_05um);
    printf("Number of particles > 1um in 0.1L: %d\n\r", pms_frame.particles_10um);
    printf("Number of particles > 2.5um in 0.1L: %d\n\r", pms_frame.particles_25um);
    printf("Number of particles > 5um in 0.1L: %d\n\r", pms_frame.particles_50um);
    printf("Number of particles > 100um in 0.1L: %d\n\r", pms_frame.particles_100um);
}

struct pms1003data pms10_get_data(void)
{
 return pms_frame;
}