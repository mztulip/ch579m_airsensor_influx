#ifndef __HTTP__
#define __HTTP__

extern char http_request_buffer[];
err_t http_prepare_request(struct tcp_pcb *tpcb);

#endif