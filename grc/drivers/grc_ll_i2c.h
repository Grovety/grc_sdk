#ifndef _GRC_LL_I2C_H_
#define _GRC_LL_I2C_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define PROTOCOL_INTERFACE_I2C 0x32220001

struct grc_ll_i2c_dev {
    uint32_t type;

    int sda_io_num;
    int scl_io_num;
    int data_ready_io_num;
    int i2c_num;
    uint32_t clk_speed;
    uint16_t slave_addr;
    uint32_t timeout_us;
};

void grc_ll_sleep(int ms);

int grc_ll_i2c_init(struct grc_ll_i2c_dev* grc);

int grc_ll_i2c_release(struct grc_ll_i2c_dev* grc);

int grc_ll_i2c_write(struct grc_ll_i2c_dev* grc, void* data, int len);

int grc_ll_i2c_read(struct grc_ll_i2c_dev* grc, void* data, int len);

// the callback is called by a pin interrupt
// Executed in the interrupt handler
// Use synchronization primitives in case of heavy or blocking  callback processing
void grc_ll_i2c_callback(struct grc_ll_i2c_dev* grc);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _GRC_LL_I2C_H_
