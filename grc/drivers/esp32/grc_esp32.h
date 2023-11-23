#ifndef _GRC_LL_I2C_H_
#define _GRC_LL_I2C_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define PROTOCOL_INTERFACE_I2C_ESP32 0x32220001

struct grc_ll_i2c_dev_esp32 {
    uint32_t type;

    int sda_io_num;
    int scl_io_num;
    int data_ready_io_num;
    int reset_io_num;
    int i2c_num;
    uint32_t clk_speed;
    uint16_t slave_addr;
    uint32_t timeout_us;
};

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _GRC_LL_I2C_H_
