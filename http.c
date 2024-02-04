
#include <stdio.h>
#include <string.h>
#include "CH57x_common.h"
#include "tcp.h"
#include "http.h"
#include "pms10.h"

// database = air_db
// measurement = pm10 or pm25 or pm100
// tag key = sensor
// tag value =sensor1
// field-key = value
const char *http_influx_post_template = "POST /write?db=air_db HTTP/1.1\x0d\x0a"
"Host: %s:%d\x0d\x0a"
"User-Agent: ch579_airsensor/1.0\x0d\x0a"
"Accept: */*\x0d\x0a"
"Content-Length: %d\x0d\x0a"
"Content-Type: application/x-www-form-urlencoded\x0d\x0a\x0d\x0a";

const char *http_post_content_template = "pm10,sensor=sensor1 value=%d\n"
"pm25,sensor=sensor1 value=%d\n"
"pm100,sensor=sensor1 value=%d";

//Buffer is greater for overflow security
static char ip_string[4*3+3+12];
char http_request_buffer[1100];
static char http_post_content_buffer[200];

static err_t ip_to_string(ip_addr_t ip_address)
{
    uint32_t ip_len = sprintf(ip_string , "%ld.%ld.%ld.%ld",  \
    ((ip_address.addr)&0x000000ff),       \
    (((ip_address.addr)&0x0000ff00)>>8),  \
    (((ip_address.addr)&0x00ff0000)>>16), \
    ((ip_address.addr)&0xff000000)>>24);
    if(ip_len > (4*3+3))
    {
        return ERR_MEM;
    }
    return ERR_OK;
}

err_t http_prepare_request(struct tcp_pcb *tpcb)
{
    err_t result;
    result = ip_to_string(tpcb->remote_ip);
    if(result != ERR_OK) {printf("\033[91buffer overflow during ip to string conversion\033\n\r");return result;}

    struct pms1003data sensor_data = pms10_get_data();
    uint32_t content_length = sprintf(http_post_content_buffer, 
    http_post_content_template, sensor_data.pm10_standard, sensor_data.pm25_standard, sensor_data.pm100_standard);
    if(content_length > 200) {printf("\033[91http request buffer overflow\033\n\r");return ERR_MEM;}

    uint32_t http_request_len = sprintf(http_request_buffer, http_influx_post_template, ip_string, tpcb->remote_port, content_length);
    if(http_request_len > 1000) {printf("\033[91http request buffer overflow\033\n\r");return ERR_MEM;}
    if((http_request_len + content_length) > 1000 ) {printf("\033[91http request content not fits in to the buffer.\033\n\r");return ERR_MEM;}
    memcpy(http_request_buffer+http_request_len, http_post_content_buffer, content_length+1);
    return ERR_OK;
}