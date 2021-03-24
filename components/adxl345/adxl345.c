#include <stdio.h>
#include <i2c.h>
#include "adxl345.h"
#define ACC (0x53)//(0xA7>>1) I2C Connection (Address 0x53) This translates to 0xA6 for a write and 0xA7 for a read.
static const char *TAG = "adxl345";

// Write val to address register on ACC
void writeTo(uint8_t DEVICE, uint8_t address, uint8_t val) {
    if (!i2c_slave_write_with_reg(DEVICE, address, val)) {
        ESP_LOGE(TAG, "I2C write error");
    }
}

void initAccelerometer(uint8_t scl_pin, uint8_t sda_pin) {
    // Turning on ADXL345
    i2c_init(scl_pin, sda_pin);
    writeTo(ACC, 0x2D, 1 << 3);
}
