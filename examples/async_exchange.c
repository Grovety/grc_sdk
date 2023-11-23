#include "grc/drivers/grc_ll_i2c.h"
#include "grc/grc.h"

#include <stdio.h>

// specify interface GRC chip connected to
static struct grc_ll_i2c_dev_esp32 ll_dev = {
    .type = PROTOCOL_INTERFACE_I2C_ESP32,
    .sda_io_num = 12,
    .scl_io_num = 13,
    .data_ready_io_num = 14,
    .i2c_num = 0,
    .clk_speed = 400000,
    .slave_addr = 0x36,
    .timeout_us = 1000
};

static struct grc_device dev = {
    .ll_dev = &ll_dev
};

static struct grc_config conf = {
    .arch = I3_N10
};

void inference_callback(int status, void* user_data)
{
    (void)status;
    (void)user_data;

    // do not perform long-running task here

    // set the event marking the inference/trainig process finished
}

void async_inference_example(float* data, int len)
{
    int res = grc_init(&dev, &conf);
    if (res < 0) {
        // report error
        return;
    }

    struct grc_inference_params i_params = {
        .flags = GRC_PARAMS_ASYNC,
        .callback = inference_callback,
        .user_data = (void*)0
    };
    res = grc_inference(&dev, &i_params, data, len);
    if (res < 0) {
        // report error
        goto out;
    }

    // wait for the event from the callback
    // or call grc_wait(&dev) instead

    struct grc_class_info info;
    for (int class_index = 0; class_index < res; class_index++) {
        grc_get_class_info_by_index(&dev, class_index, &info);

        printf("Tag: %u :", info.tag);
        for (uint32_t i = 0; i < info.responce_len; i++)
            printf("%f, ", info.responce[i]);
        printf("\n");
    }

out:
    grc_release(&dev);
}
