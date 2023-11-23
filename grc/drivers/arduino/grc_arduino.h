#ifndef _GRC_DRIVERS_I2C_ARDUINO_H_
#define _GRC_DRIVERS_I2C_ARDUINO_H_

#include <stdint.h>
#include <Wire.h>


#define PROTOCOL_INTERFACE_I2C_ARDUINO 0x32220002

struct grc_ll_i2c_dev_arduino {
    uint32_t type;
    TwoWire* arduino_wire;
    int reset_pin;
};


#endif // _GRC_DRIVERS_I2C_ARDUINO_H_
