#ifndef __PMS10__
#define __PMS10__

#include <stdint.h>

struct __attribute__((packed, scalar_storage_order("big-endian"))) pms1003data 
{
    uint16_t start_bytes;
    uint16_t frame_len;
    uint16_t pm10_standard, pm25_standard, pm100_standard;
    uint16_t pm1_env, pm25_env, pm_env;
    uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
    uint16_t unused;
    uint16_t checksum;
};

void pms10_init(void);
void pms10_read(void);
void pms10_print_data(void);
struct pms1003data pms10_get_data(void);

#endif