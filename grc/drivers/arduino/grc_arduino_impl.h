#ifndef _GRC_DRIVERS_I2C_ARDUINO_IMPL_H_
#define _GRC_DRIVERS_I2C_ARDUINO_IMPL_H_

#include "grc_arduino.h"
#include "grc/grc_error_codes.h"


#define GRC_AI_MODULE_FREQ_HZ 400000
#define GRC_AI_MODULE_I2C_ADDR 0x36


extern "C" void grc_ll_sleep(int ms)
{
    delay(ms);
}

extern "C" int grc_ll_i2c_init(void* dev)
{
    grc_ll_i2c_dev_arduino* ll_dev = reinterpret_cast<grc_ll_i2c_dev_arduino*>(dev);
    if (ll_dev->type != PROTOCOL_INTERFACE_I2C_ARDUINO)
        return ARGUMENT_ERROR;

    ll_dev->arduino_wire->begin();
    ll_dev->arduino_wire->setClock(GRC_AI_MODULE_FREQ_HZ);
    ll_dev->arduino_wire->setTimeOut(1);
    ll_dev->arduino_wire->setBufferSize(256);
    return GRC_OK;
}

extern "C" int grc_ll_i2c_release(void* dev)
{
    grc_ll_i2c_dev_arduino* ll_dev = reinterpret_cast<grc_ll_i2c_dev_arduino*>(dev);
    if (ll_dev->type != PROTOCOL_INTERFACE_I2C_ARDUINO)
        return ARGUMENT_ERROR;

    ll_dev->arduino_wire->end();
    return GRC_OK;
}

extern "C" int grc_ll_i2c_write(void* dev, void* data, int len)
{
    if (len < 1)
        return ARGUMENT_ERROR;

    grc_ll_i2c_dev_arduino* ll_dev = reinterpret_cast<grc_ll_i2c_dev_arduino*>(dev);
    if (ll_dev->type != PROTOCOL_INTERFACE_I2C_ARDUINO)
        return ARGUMENT_ERROR;

    ll_dev->arduino_wire->beginTransmission(GRC_AI_MODULE_I2C_ADDR);
    ll_dev->arduino_wire->write(reinterpret_cast<const uint8_t*>(data), len);
    ll_dev->arduino_wire->endTransmission(true);
    return len;
}

extern "C" int grc_ll_i2c_read(void* dev, void* data, int len)
{
    if (len < 1)
        return ARGUMENT_ERROR;

    grc_ll_i2c_dev_arduino* ll_dev = reinterpret_cast<grc_ll_i2c_dev_arduino*>(dev);
    if (ll_dev->type != PROTOCOL_INTERFACE_I2C_ARDUINO)
        return ARGUMENT_ERROR;

    ll_dev->arduino_wire->requestFrom((uint8_t)GRC_AI_MODULE_I2C_ADDR, (size_t)len, true);

    int readed = 0;
    uint8_t* buf = reinterpret_cast<uint8_t*>(data);
    while (readed < len && ll_dev->arduino_wire->available()) {
        buf[readed++] = ll_dev->arduino_wire->read();
    }
    return len;
}

void grc_ll_i2c_callback(void* dev)
{
}

int grc_ll_gpio_init(void* dev)
{
    grc_ll_i2c_dev_arduino* ll_dev = (grc_ll_i2c_dev_arduino*)dev;
    if (ll_dev->type != PROTOCOL_INTERFACE_I2C_ARDUINO)
        return ARGUMENT_ERROR;

    pinMode(ll_dev->reset_pin, OUTPUT);

    return GRC_OK;
}

int grc_ll_gpio_reset_high(void* dev)
{
    grc_ll_i2c_dev_arduino* ll_dev = (grc_ll_i2c_dev_arduino*)dev;
    if (ll_dev->type != PROTOCOL_INTERFACE_I2C_ARDUINO)
        return ARGUMENT_ERROR;

    digitalWrite(ll_dev->reset_pin, HIGH);

    return GRC_OK;
}

int grc_ll_gpio_reset_low(void* dev)
{
    grc_ll_i2c_dev_arduino* ll_dev = (grc_ll_i2c_dev_arduino*)dev;
    if (ll_dev->type != PROTOCOL_INTERFACE_I2C_ARDUINO)
        return ARGUMENT_ERROR;

    digitalWrite(ll_dev->reset_pin, LOW);

    return GRC_OK;
}

#endif // _GRC_DRIVERS_I2C_ARDUINO_IMPL_H_
