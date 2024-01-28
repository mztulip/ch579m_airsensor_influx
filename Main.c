#include "CH57x_common.h"
#include <stdio.h>
#include <string.h>
#include "eth_mac.h"
#include "ethernetif.h"
#include "timer0.h"
#include "lwipcomm.h"
#include "lwip/timeouts.h"
#include "tcp.h"

uint32_t arg = 0;

void uart_init(void)		
{
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
}

void led_init(void)
{
    GPIOB_ModeCfg( GPIO_Pin_0, GPIO_ModeOut_PP_20mA );
}

void led_on(void)
{
    GPIOB_SetBits( GPIO_Pin_0 ); 
}

void led_off(void)
{
    GPIOB_ResetBits( GPIO_Pin_0 );
}

static err_t tcp_influx_received(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    printf("\033[32m Data received.\n\r\033[0m");

    // Remote client closes connection
    if (p == NULL)
    {
        printf("Closing connection, request client\n\r");
        tcp_close(tpcb);

        return ERR_OK;
    }
    else if(err != ERR_OK)
    {
        printf("\033[91mReceived error.\033[0m1\n\r");
        return err;
    }
    printf("Received Data len: %d\n\r", p->tot_len);
    printf("Data: ");
    for (int i = 0; i < p->tot_len; i++)
    {
        uint8_t byte = *(uint8_t*)(p->payload+i);
        printf("%c ", byte);
    }
    printf("\n\r");

    tcp_recved(tpcb, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}

static void tcp_influx_error(void *arg, err_t err)
{
  printf("\033[91mtcp connection fatal Error. Maybe memory shortage.\033[0m\n\r");
}

void influxdb_connect(void)
{
    static struct tcp_pcb *tcp_pcb_handle;
    err_t result;

    tcp_pcb_handle = tcp_new();
    if(tcp_pcb_handle == NULL){printf("tcp_new failed\n\r");return;}

    tcp_arg(tcp_pcb_handle, &arg);

    tcp_recv(tcp_pcb_handle, tcp_influx_received);
    tcp_err(tcp_pcb_handle, tcp_influx_error);
    tcp_sent(tcp_pcb_handle, tcp_influx_sent);
}

// Very helpful link https://lwip.fandom.com/wiki/Raw/TCP
int main()
{ 
	SystemInit();

    PWR_UnitModCfg(ENABLE, (UNIT_SYS_PLL|UNIT_ETH_PHY)); 
    DelayMs(3); 
    SetSysClock(CLK_SOURCE_HSE_32MHz); 

    led_init();
    InitTimer0();
    uart_init();
    printf("\n\rAirsensor.\n\r");
    lwip_comm_init(); 

    influxdb_connect();

    while(1)
    {
        lwip_pkt_handle();
        lwip_periodic_handle();
        sys_check_timeouts();	
    }
}
