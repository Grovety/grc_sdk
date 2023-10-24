#include "grc/drivers/grc_ll_i2c.h"

int grc_ll_i2c_init(struct grc_ll_i2c_dev* dev, int id)
{
    (void)dev;
    (void)id;
    return -1;
}

int grc_ll_i2c_release(struct grc_ll_i2c_dev* dev)
{
    (void)dev;
    return -1;
}

int grc_ll_i2c_write(struct grc_ll_i2c_dev* dev, void* data, int len)
{
    (void)dev;
    (void)data;
    (void)len;
    return -1;
}

int grc_ll_i2c_read(struct grc_ll_i2c_dev* dev, void* data, int len)
{
    (void)dev;
    (void)data;
    (void)len;
    return -1;
}

void grc_ll_i2c_callback(struct grc_ll_i2c_dev* dev)
{
    (void)dev;
}
