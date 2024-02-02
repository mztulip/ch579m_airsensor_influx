#include "CH57x_common.h"
#include <stdio.h>
#include <string.h>
#include "eth_mac.h"
#include "ethernetif.h"
#include "timer0.h"
#include "lwipcomm.h"
#include "lwip/timeouts.h"
#include "tcp.h"
#include "pms10.h"

uint32_t arg = 0;
static struct tcp_pcb *tcp_pcb_handle = NULL;

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
    err_t result;
    printf("\033[32m Data received.\n\r\033[0m");
    // Remote server closes connection
    if (p == NULL)
    {
        printf("Connection closed\n\r");
        result = tcp_close(tpcb);
        if(result != ERR_OK) {printf("tcp_close error \n\r");}
        if(result == ERR_MEM) {printf("tcp_close can not allocate memory\n\r");}
        tcp_pcb_handle = NULL;
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
        printf("%c", byte);
    }
    printf("\n\r");

    tcp_recved(tpcb, p->tot_len);
    pbuf_free(p);
  
    return ERR_OK;
}

static void tcp_influx_error(void *arg, err_t err)
{
    if( err == ERR_RST)
    {
        printf("\033[91mconnection was reset by remote server\033\n\r");
    }
    else if (err == ERR_ABRT )
    {
        printf("Connection aborted\n\r");
    }
    else 
    {
        printf("\033[91mtcp connection fatal: %s\033[0m\n\r", lwip_strerr(err));
    }
    tcp_pcb_handle = NULL;
}

static err_t tcp_influx_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    printf("Packet sent.\n\r");
    return ERR_OK;
}

char *http_post = "POST /write?db=woda_db HTTP/1.1\x0d\x0a"
"Host: 192.168.2.101:8086\x0d\x0a"
"User-Agent: ch579_airsensor/1.0\x0d\x0a"
"Accept: */*\x0d\x0a"
"Content-Length: 29\x0d\x0a"
"Content-Type: application/x-www-form-urlencoded\x0d\x0a\x0d\x0a"
"licznik,kto=woda1 value=45301";

void influx_tcp_send_packet(struct tcp_pcb *tpcb)
{
    err_t result;
    printf("Send queue: %d\n\r", tcp_sndqueuelen(tpcb));
    result = tcp_write(tpcb, http_post, strlen(http_post), TCP_WRITE_FLAG_COPY);
    if(result != ERR_OK) {printf("tcp_write error \n\r");return;}
    result = tcp_output(tpcb);
    if(result != ERR_OK) {printf("tcp_output error \n\r");return;}
}

static err_t tcp_connection_poll(void *arg, struct tcp_pcb *tpcb)
{
    printf("\033[33mTCP poll\n\r\033[0m");
    return ERR_OK;
}

err_t tcp_influx_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    tcp_poll(tpcb, tcp_connection_poll, 10);
    printf("Connected.\n\r");
    return ERR_OK;
}

void influxdb_connect(void)
{
    err_t result;
 
    tcp_pcb_handle = tcp_new();
    if(tcp_pcb_handle == NULL){printf("tcp_new failed, no memory\n\r");return;}

    tcp_arg(tcp_pcb_handle, &arg);
    tcp_recv(tcp_pcb_handle, tcp_influx_received);
    tcp_err(tcp_pcb_handle, tcp_influx_error);
    tcp_sent(tcp_pcb_handle, tcp_influx_sent);
 
    ip_addr_t influx_server_ip;
    IP4_ADDR(&influx_server_ip, 192,168,2,101);  

    result = tcp_connect(tcp_pcb_handle, &influx_server_ip, 8086, tcp_influx_connected);
    if(result != ERR_OK) {printf("tcp_connect error \n\r");}
    if(result == ERR_MEM) {printf("tcp_connect can not allocate memory \n\r");}
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

    struct Timer0Delay sendTimer;
    timer0_init_wait_10ms(&sendTimer, 500); //every 5s

    pms10_init();

    uint8_t buffer[100];
    
    while(1)
    {
        uint32_t len = UART3_RecvString(buffer);
        if(len > 0)
        {
            printf("\n\rLen:%d\n\r", len);
            for(int i = 0 ; i < len; i++)
            {
                printf("%02x ", buffer[i]);
            }
        }
        // if(tcp_pcb_handle == NULL)
        // {
        //     influxdb_connect();
        // }
        // if(timer0_check_wait(&sendTimer) && tcp_pcb_handle != NULL)
        // {
        //     printf("state: %s\n\r", tcp_debug_state_str(tcp_pcb_handle->state));
        //     if(tcp_pcb_handle->state == ESTABLISHED)
        //     {
        //         influx_tcp_send_packet(tcp_pcb_handle);
        //     }
        // }
        // lwip_pkt_handle();
        // lwip_periodic_handle();
        // sys_check_timeouts();	
    }
}

