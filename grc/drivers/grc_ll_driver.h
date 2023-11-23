#ifndef _GRC_LL_DRIVER_H_
#define _GRC_LL_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void grc_ll_sleep(int ms);

int grc_ll_i2c_init(void* dev);
int grc_ll_i2c_release(void* dev);
int grc_ll_i2c_write(void* dev, void* data, int len);
int grc_ll_i2c_read(void* dev, void* data, int len);
void grc_ll_i2c_callback(void* dev);

int grc_ll_gpio_init(void* dev);
int grc_ll_gpio_reset_high(void* dev);
int grc_ll_gpio_reset_low(void* dev);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _GRC_LL_DRIVER_H_
