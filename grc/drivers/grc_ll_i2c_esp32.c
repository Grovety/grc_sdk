#include "grc/grc_error_codes.h"
#include "grc/drivers/grc_ll_i2c.h"

#include "driver/i2c.h"
#include "sdkconfig.h"

// ===== base esp32-i2c params ===========
#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define WRITE_BIT I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ /*!< I2C master read */
#define ACK_CHECK_EN 0x1 /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0 /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0 /*!< I2C ack value */
#define NACK_VAL 0x1 /*!< I2C nack value */
// ========================================================

// ====== default i2c values for dev board ===========
#define I2C_MASTER_SDA_IO GPIO_NUM_12
#define I2C_MASTER_SCL_IO GPIO_NUM_13
#define I2C_DATA_READY_PIN GPIO_NUM_14

#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 400000
#define ESP_SLAVE_ADDR 0x36

#define SLAVE_REQUEST_WAIT_MS (1000 / portTICK_PERIOD_MS)
//==================================================

struct grc_i2c_esp32_config {
    int sda_io_num; // GPIO number for I2C sda signal
    int scl_io_num; // GPIO number for I2C scl signal
    int data_ready_io_num; // GPIO number for GRC data ready signal
    i2c_port_t i2c_num; // I2C port number, can be I2C_NUM_0 ~ (I2C_NUM_MAX-1).
    uint32_t clk_speed; // I2C clock frequency for master mode, (no higher than 1MHz for now)
    uint16_t slave_addr; // I2C address for slave mode
    TickType_t ticks_to_wait; // Maximum ticks to wait before issuing a timeout
};

#define ESP32_CONFIG_COUNT 1
const struct grc_i2c_esp32_config esp32_i2c_config_array[] = {
    { .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .data_ready_io_num = I2C_DATA_READY_PIN,
        .i2c_num = I2C_MASTER_NUM,
        .clk_speed = I2C_MASTER_FREQ_HZ,
        .slave_addr = ESP_SLAVE_ADDR,
        .ticks_to_wait = SLAVE_REQUEST_WAIT_MS

    }
};

#define CHECK_I2C_RESULT(func)    \
    {                             \
        esp_err_t retcode = func; \
        if (retcode != ESP_OK) {  \
            return I2C_ERROR;     \
        }                         \
    }

void grc_ll_sleep(int ms)
{
    vTaskDelay((ms) / portTICK_PERIOD_MS);
}

int grc_ll_i2c_init(struct grc_ll_i2c_dev* dev)
{

    i2c_port_t i2c_master_port = dev->i2c_num;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = dev->sda_io_num;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = dev->scl_io_num;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = dev->clk_speed;
    ;
    conf.clk_flags = 0;
    CHECK_I2C_RESULT(i2c_param_config(i2c_master_port, &conf))
    CHECK_I2C_RESULT(i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0))
    return I2C_OK;
}

int grc_ll_i2c_release(struct grc_ll_i2c_dev* dev)
{
    CHECK_I2C_RESULT(i2c_driver_delete(dev->i2c_num))
    return I2C_OK;
}

int grc_ll_i2c_write(struct grc_ll_i2c_dev* dev, void* data, int len)
{
    uint8_t *p8 = data;
    if (len < 1) {
        return ARGUMENT_ERROR;
    }
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (!cmd) {
        return I2C_ERROR;
    }
    CHECK_I2C_RESULT(i2c_master_start(cmd))
    CHECK_I2C_RESULT(i2c_master_write_byte(cmd, (dev->slave_addr << 1) | WRITE_BIT, ACK_CHECK_EN))
    CHECK_I2C_RESULT(i2c_master_write(cmd, p8, len, ACK_CHECK_EN))
    CHECK_I2C_RESULT(i2c_master_stop(cmd))
    CHECK_I2C_RESULT(i2c_master_cmd_begin(dev->i2c_num, cmd, dev->timeout_us))
    i2c_cmd_link_delete(cmd);
    return len;
}

int grc_ll_i2c_read(struct grc_ll_i2c_dev* dev, void* data, int len)
{
    uint8_t *p8 = data;
    if (len < 1) {
        return ARGUMENT_ERROR;
    }
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (!cmd) {
        return I2C_ERROR;
    }
    CHECK_I2C_RESULT(i2c_master_start(cmd))
    CHECK_I2C_RESULT(i2c_master_write_byte(cmd, (dev->slave_addr << 1) | READ_BIT, ACK_CHECK_EN))
    if (len > 1) {
        CHECK_I2C_RESULT(i2c_master_read(cmd, p8, len - 1, (i2c_ack_type_t)ACK_VAL))
    }

    CHECK_I2C_RESULT(i2c_master_read_byte(cmd, &p8[len - 1], (i2c_ack_type_t)NACK_VAL))
    CHECK_I2C_RESULT(i2c_master_stop(cmd))
    CHECK_I2C_RESULT(i2c_master_cmd_begin(dev->i2c_num, cmd, dev->timeout_us))
    i2c_cmd_link_delete(cmd);
    return len;
}

void grc_ll_i2c_callback(struct grc_ll_i2c_dev* dev)
{
}
