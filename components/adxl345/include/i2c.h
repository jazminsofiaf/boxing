#include "driver/gpio.h"
#include "esp_log.h"
#include "esp32/rom/ets_sys.h"

// Init bitbanging I2C driver on given pins
void i2c_init(uint8_t scl_pin, uint8_t sda_pin);

// Write a byte to I2C bus. Return true if slave acked.
bool i2c_write(uint8_t byte);

bool i2c_slave_write_with_reg(uint8_t slave_addr,uint8_t reg_addr, uint8_t data);


// Send start and stop conditions. Only needed when implementing protocols for
// devices where the i2c_slave_[read|write] functions above are of no use.
void i2c_start(void);
void i2c_stop(void);

